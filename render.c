#include "render.h"
#include "transform.h"
#include <SDL2/SDL.h>

// one of each for drawing -- nothing more complicated required
SDL_Window *win = NULL;
SDL_Surface *surface = NULL;
SDL_Renderer *ren = NULL;
SDL_Texture *tex = NULL;

void log_SDL_error(const char *e) {
  printf("%s failed:\n\t%s\n", e, SDL_GetError());
}

int init_live_render(int w, int h) {

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
  SDL_RenderClear(ren);
  rendering_initialized = 1;
  return 0;
}

// set and get pix from the surface
void setpix(int x, int y, uint32_t color, char lock) {
  if (x < 0 || y < 0 || x > surface->w || y > surface->h)
    return;

  if (lock && SDL_MUSTLOCK(surface))
    SDL_LockSurface(surface);

  uint8_t * p = (uint8_t *)surface->pixels;
  p += (y * surface->pitch) + (x * sizeof(uint32_t));
  *((uint32_t *)p) = color;

  if (lock && SDL_MUSTLOCK(surface))
    SDL_UnlockSurface(surface);
}
uint32_t getpix(int x, int y, char lock) {
  if (lock && SDL_MUSTLOCK(surface))
    SDL_LockSurface(surface);

  uint8_t *p = (uint8_t *)surface->pixels;
  p += (y * surface->pitch) + (x * sizeof(uint32_t));

  if (lock && SDL_MUSTLOCK(surface))
    SDL_UnlockSurface(surface);

  return *((uint32_t *)p);
}

// scale xy of world coordinates to fit screen
// screen = {xmin, ymin, xmax, ymax};
// coors = {x1, y1, x2, y2};
void map_coors(double *coors, int *mapped) {
  double sw = screen[2] - screen[0], sh = screen[3] - screen[1];
  double xscale = surface->w/sw, yscale = surface->h/sh;

  mapped[0] = ceil((coors[0] - screen[0])*xscale);
  mapped[1] = -ceil((coors[1] - screen[3])*yscale);
  mapped[2] = ceil((coors[2] - screen[0])*xscale);
  mapped[3] = -ceil((coors[3] - screen[3])*yscale);
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
  if (SDL_MUSTLOCK(surface))
    SDL_LockSurface(surface);
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
  if (SDL_MUSTLOCK(surface))
    SDL_UnlockSurface(surface);
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
  if (SDL_MUSTLOCK(surface))
    SDL_LockSurface(surface);
  for (y = 0; y < surface->h; y++) {
    for (x = 0; x < surface->w; x++) {
      p = getpix(x, y, 0);
      SDL_GetRGB(p, surface->format, &r, &g, &b);
      bwrit += sprintf(obuf+bwrit, "%d %d %d\t", r, g, b);
    }
  }
  if (SDL_MUSTLOCK(surface))
    SDL_UnlockSurface(surface);
  fwrite(obuf, 1, bwrit, out);
  fclose(out);
}

void renderparallel(Matrix *edge) {
  clear_screen();
  int mapped[4];
  double unmapped[4];
  uint32_t color = SDL_MapRGB(surface->format, 255, 255, 255);
  int c;
  for (c = 0; c < edge->cols; c += 2) {
    unmapped[0] = mat_get_cell(edge, c, 0);
    unmapped[1] = mat_get_cell(edge, c, 1);
    unmapped[2] = mat_get_cell(edge, c+1, 0);
    unmapped[3] = mat_get_cell(edge, c+1, 1);
    map_coors(unmapped, mapped);
    dline(mapped, color);
  }
  update_display();
}

void renderperspective(Matrix *edge, double *eye, uint32_t color) {
	int mapped[4];
	double unmapped[4];
	int c;
	double ex = eye[0], ey = eye[1], ez = eye[2];
	//printf("rendering perspective: %.2f %.2f %.2f\n", ex, ey, ez);
	double pz;
	for (c = 0; c < edge->cols; c += 2) {
		pz = mat_get_cell(edge, c, 2);
    if (pz > ez)
      break;
		unmapped[0] = ex - (ez * (mat_get_cell(edge, c, 0)-ex) / (pz - ez));
		unmapped[1] = ey - (ez * (mat_get_cell(edge, c, 1)-ey) / (pz - ez));
		pz = mat_get_cell(edge, c+1, 2);
    if (pz > ez)
      break;
		unmapped[2] = ex - (ez * (mat_get_cell(edge, c+1, 0)-ex) / (pz - ez));
		unmapped[3] = ey - (ez * (mat_get_cell(edge, c+1, 1)-ey) / (pz - ez));
		map_coors(unmapped, mapped);
		dline(mapped, color);
	}
}

void rendercyclops(Matrix *edge, double *eye) {
  clear_screen();
  renderperspective(edge, eye, SDL_MapRGB(surface->format, 255, 255, 255));
  update_display();
}

void renderstereo(Matrix *edge, double *eyes) {
  clear_screen();

  // left -- red
  renderperspective(edge, eyes, SDL_MapRGB(surface->format, 127, 0, 0));

  // right -- cyan
  renderperspective(edge, eyes+3, SDL_MapRGB(surface->format, 0, 127, 127));

  update_display();
}

void update_display() {
  SDL_UpdateTexture(tex, NULL, surface->pixels, surface->pitch);
  SDL_RenderCopy(ren, tex, NULL, NULL);
  SDL_RenderPresent(ren);
}

void clear_screen() {
  SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
  update_display();
}

void spin(Matrix *edge) {
  Matrix *x = rotate_x_mat(1 * M_PI / 180);
  Matrix *y = rotate_y_mat(1 * M_PI / 180);
  Matrix *z = rotate_z_mat(1 * M_PI / 180);
  Matrix *temp = mat_multiply(x, y);
  Matrix *xyz = mat_multiply(temp, z);
  mat_destruct(temp);
  mat_destruct(x);
  mat_destruct(y);
  mat_destruct(z);
  double eye[3] = {0, 0, 3};
  Matrix *rot;
  uint32_t color = SDL_MapRGB(surface->format, 0, 200, 0);
  uint32_t black = SDL_MapRGB(surface->format, 0, 0, 0);
  clear_screen();
  renderperspective(edge, eye, color);
  while(!should_quit()) {
    renderperspective(edge, eye, black);
    rot = mat_multiply(xyz, edge);
    mat_destruct(edge);
    edge = rot;
    renderperspective(edge, eye, color);
    SDL_Delay(50);
    update_display();
  }
}

// call to clean up
void finish_live_render() {
  SDL_DestroyWindow(win);
  SDL_DestroyRenderer(ren);
  SDL_DestroyTexture(tex);
  SDL_FreeSurface(surface);
  SDL_Quit();
}

char should_quit() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if ( event.type == SDL_QUIT ) {
      SDL_DestroyRenderer(ren);
      SDL_DestroyWindow(win);
      SDL_Quit();
      return 1;
    }
  }
  return 0;
}
