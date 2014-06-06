#include "display.h"
#include "transform.h"
#include <SDL2/SDL.h>

// one of each for drawing -- nothing more complicated required
SDL_Window *win = NULL;
SDL_Surface *surface = NULL;
SDL_Renderer *ren = NULL;
SDL_Texture *tex = NULL;

// the kz_buf for lighting + z-buf (to later be pushed to screen)
KZ_Point **kz_buf;

// should mix colors when adding new pixel
char mix = 0;

char ambient_red = 255;
char ambient_green = 255;
char ambient_blue = 255;

void log_SDL_error(const char *e) {
  printf("%s failed:\n\t%s\n", e, SDL_GetError());
}

int init_live_render(int w, int h) {
  // init kz_buf
  kz_buf = malloc(w * sizeof(KZ_Point *));
  int i, j;
  for (i = 0; i < w; i++) {
    kz_buf[i] = calloc(h, sizeof(KZ_Point));
    for (j = 0; j < h; j++) {
      kz_buf[i][j] = (KZ_Point){-1, -1, 0.0, 0.0, 0.0, 0.0};
    }
  }
  if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
    log_SDL_error("SDL_Init()");
    return 1;
  }
  win = SDL_CreateWindow("Live Rendering 3D Graphics",
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, w, h,
                         SDL_WINDOW_SHOWN);
  if (!win) {
    log_SDL_error("SDL_CreateWindow()");
    return 1;
  }
  ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED |
                           SDL_RENDERER_PRESENTVSYNC);
  if (!ren) {
    log_SDL_error("SDL_CreateRenderer()");
    return 1;
  }
  surface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
  if (!surface) {
    log_SDL_error("SDL_CreateRGBSurface()");
    return 1;
  }
  tex = SDL_CreateTextureFromSurface(ren, surface);
  if (!tex) {
    log_SDL_error("SDL_CreateTextureFromSurface()");
    return 1;
  }
  clear_screen();
  rendering_initialized = 1;
  return 0;
}

void mixcolors(char b) {
  mix = b;
}

char pix_in_screen(int x, int y) {
  return x > 0 && y > 0 && x < surface->w && y < surface->h;
}

// set and get pix from the surface
void setpix(int x, int y, uint32_t color, char lock) {
  if (x < 0 || y < 0 || x > surface->w || y > surface->h)
    return;

  if (lock)
    lock_surface();

  if (mix)
    color += getpix(x, y, 0);
  uint8_t * p = (uint8_t *)surface->pixels;
  p += (y * surface->pitch) + (x * sizeof(uint32_t));
  *((uint32_t *)p) = color;

  if (lock)
    unlock_surface();
}

uint32_t getpix(int x, int y, char lock) {
  if (lock)
    lock_surface();

  uint8_t *p = (uint8_t *)surface->pixels;
  p += (y * surface->pitch) + (x * sizeof(uint32_t));

  if (lock)
    unlock_surface();

  return *((uint32_t *)p);
}

// inserts this point into the kz buffer if it isn't occluded
void consider_KZ_Point(KZ_Point p) {
  if (p.x >= 0 && p.x < surface->w && p.y < surface->h
      && (kz_buf[p.x][p.y].x < 0 || kz_buf[p.x][p.y].r >= p.r)) {
    kz_buf[p.x][p.y] = p;
  }
}

void flip_KZ_buffer() {
  lock_surface();
  int x, y;
  uint32_t color;
  for (x = 0; x < surface->w; x ++) {
    for (y = 0; y < surface->h; y++) {
      if (kz_buf[x][y].x < 0) continue;
      color = rgb(kz_buf[x][y].kr * ambient_red,
		  kz_buf[x][y].kg * ambient_green,
		  kz_buf[x][y].kb * ambient_blue);
	setpix(x, y, color, 0);
    }
  }
  unlock_surface();
}


void lock_surface() {
  if (SDL_MUSTLOCK(surface))
    SDL_LockSurface(surface);
}

void unlock_surface() {
  if (SDL_MUSTLOCK(surface))
    SDL_UnlockSurface(surface);
}

uint32_t rgb(int r, int g, int b) {
  return SDL_MapRGB(surface->format, r, g, b);
}

// scale xy of world coordinates to fit screen
// screen = {xmin, ymin, xmax, ymax};
void map_coors(double *x, double *y) {
  double sw = screen[2] - screen[0], sh = screen[3] - screen[1];
  double xscale = surface->w/sw, yscale = surface->h/sh;

  *x = ceil((*x - screen[0])*xscale);
  *y = -ceil((*y - screen[3])*yscale);
}

// draw a line
void dline(int *coors, uint32_t color) {
  // draw a line to out
  int x1, x2, y1, y2;
  // ensure left to right
  if (coors[0] > coors[2]) {
    x1 = coors[2];
    x2 = coors[0];
    y1 = coors[3];
    y2 = coors[1];
  } else {
    x2 = coors[2];
    x1 = coors[0];
    y2 = coors[3];
    y1 = coors[1];
  }
  int dx = x2 - x1, dy = y2 > y1?y2 - y1:y1 - y2;
  char xMaj = dx > dy;
  int x = x1, y = y1;
  int acc = dx/2;
  int step = y1 < y2 ? 1 : -1;
  lock_surface();
  if (xMaj) {
    while (x <= x2) {
      setpix(x, y, color, 0);
      acc -= dy;
      if (acc < 0) {
        y += step;
        acc += dx;
      }
      x++;
    }
  } else {
    int acc = dy/2;
    char up = y1 < y2;
    while (up ? y <= y2 : y >= y2) {
      setpix(x, y, color, 0);
      acc -= dx;
      if (acc < 0) {
        x++;
        acc += dy;
      }
      y += step;
    }
  }
  unlock_surface();
}

// rendering functions
void renderppm(char *path) {
  FILE *out = fopen(path, "w");
  fprintf(out, "P3 %d %d 255\n", surface->w, surface->h);
  int maxb = 13 * surface->w * surface->h;
  char *obuf = malloc(maxb);
  uint32_t p = 0;
  int x, y, bwrit = 0;
  unsigned char r = 0, g = 0, b = 0;
  lock_surface();
  for (y = 0; y < surface->h; y++) {
    for (x = 0; x < surface->w; x++) {
      p = getpix(x, y, 0);
      SDL_GetRGB(p, surface->format, &r, &g, &b);
      bwrit += sprintf(obuf+bwrit, "%d %d %d\t", r, g, b);
    }
  }
  unlock_surface();
  fwrite(obuf, 1, bwrit, out);
  fclose(out);
}

void update_display() {
  SDL_UpdateTexture(tex, NULL, surface->pixels, surface->pitch);
  SDL_RenderCopy(ren, tex, NULL, NULL);
  SDL_RenderPresent(ren);
}

void clear_screen() {
  clear_pixel_buffer();
  update_display();
}

void clear_pixel_buffer() {
  SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
  int i, j;
  for (i = 0; i < surface->w; i++) {
    for (j = 0; j < surface->h; j++) {
      kz_buf[i][j] = (KZ_Point){-1, -1, 0, 0, 0, 0};
    }
  }
}

// call to clean up
void finish_live_display() {
  SDL_DestroyWindow(win);
  SDL_DestroyRenderer(ren);
  SDL_DestroyTexture(tex);
  SDL_FreeSurface(surface);
  SDL_Quit();
}

char should_quit() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      return 1;
    }
  }
  return 0;
}
