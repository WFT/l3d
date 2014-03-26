 #include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "render.h"
#include "matrix.h"
#include "parse_util.h"
#include "transform.h"

#define TO_RAD(deg) (deg * M_PI / 180)

Matrix *edge;
Matrix *tform;
char quit = 0;
char sdl_initialized = 0;
FILE *in;

void multiply_transform(Matrix *transform) {
  mat_multinmat(transform, tform, tform);
}

void interpret(char *l) {
  if (l[0] == '#')
    return;
  char **list = parse_split(l);
  int argc = parse_numwords(list) - 1;
  double *args = calloc(argc, sizeof(double));
  int i;
  for (i = 0; i < argc; i++) {
    args[i] = strtod(list[i+1], NULL);
  }
  if (strcmp(list[0], "pixels") == 0) {
    // quit on error
    quit = init_live_render(args[0], args[1]);
    if (quit)
      printf("live rendering setup failed... Exiting now.\n");
  } else if (strcmp(list[0], "screen") == 0) {
    screen = malloc(4 * sizeof(double));
    memcpy(screen, args, 4 * sizeof(double));
  } else if (strcmp(list[0], "line") == 0) {
    double *s = malloc(4 * sizeof(double));
    s[0] = args[0];
    s[1] = args[1];
    s[2] = args[2];
    s[3] = 1;

    double *e = malloc(4 * sizeof(double));
    e[0] = args[3];
    e[1] = args[4];
    e[2] = args[5];
    e[3] = 1;
    mat_add_column(edge, s);
    mat_add_column(edge, e);
  } else if (strcmp(list[0], "sphere") == 0) {
    double r = args[0], cx = args[1], cy = args[2], cz = args[3];
    int nVertices = 36;
    double lrad = 2 * M_PI / (nVertices);
    Matrix *roty = rotate_y_mat(lrad);
    Matrix *sphere = mat_construct(0, 4);
    Matrix *arc = mat_construct(0, 4);
    // gen original arc
    double coors[4] = {0, 0, 0, 1};
    int i;
    for (i = 0; i < nVertices; i++) {
      coors[0] = r * cos(i * lrad);
      coors[1] = r * sin(i * lrad);
      mat_add_column(arc, coors);

      coors[0] = r * cos((i+1) * lrad);
      coors[1] = r * sin((i+1) * lrad);
      mat_add_column(arc, coors);
    }
    Matrix *oldarc = arc;
    mat_extend(sphere, arc);
    for (i = 0; i < nVertices; i++) {
      arc = mat_multiply(roty, oldarc);
      mat_extend(sphere, arc);
      int j;
      for (j = 1; j < oldarc->cols; j += 2) {
        mat_add_column(sphere, oldarc->cells[j]);
        mat_add_column(sphere, arc->cells[j]);
      }
      mat_destruct(oldarc);
      oldarc = arc;
    }
    Matrix *center = move_mat(cx, cy, cz);
    Matrix *complete = mat_multiply(center, sphere);
    mat_extend(edge, complete);
    mat_destruct(sphere);
    mat_destruct(center);
    mat_destruct(complete);
  } else if (strcmp(list[0], "identity") == 0) {
    tform = identity_mat();
  } else if (strcmp(list[0], "move") == 0) {
    multiply_transform(move_mat(args[0], args[1], args[2]));
  } else if (strcmp(list[0], "scale") == 0) {
    multiply_transform(scale_mat(args[0], args[1], args[2]));
  } else if (strcmp(list[0], "rotate-x") == 0) {
    multiply_transform(rotate_x_mat(TO_RAD(args[0])));
  } else if (strcmp(list[0], "rotate-y") == 0) {
    multiply_transform(rotate_y_mat(TO_RAD(args[0])));
  } else if (strcmp(list[0], "rotate-z") == 0) {
    multiply_transform(rotate_z_mat(TO_RAD(args[0])));
  } else if (strcmp(list[0], "transform") == 0) {
    Matrix *temp = mat_multiply(tform, edge);
    edge = temp;
    clear_screen();
  } else if (strcmp(list[0], "clear-edges") == 0) {
    mat_destruct(edge);
    edge = mat_construct(0, 4);
  } else if (strcmp(list[0], "clear-pixels") == 0) {
    clear_screen();
  } else if (strcmp(list[0], "render-parallel") == 0) {
    if (!screen) {
      printf("ERROR: screen not set\n");
      return;
    }
    renderparallel(edge);
  } else if (strcmp(list[0], "pedge") == 0) {
    pmat(edge);
  } else if (strcmp(list[0], "render-perspective-cyclops") == 0) {
    if (!screen) {
      printf("ERROR: screen not set\n");
      return;
    }
    rendercyclops(edge, args);
  } else if (strcmp(list[0], "render-perspective-stereo") == 0) {
    if (!screen) {
      printf("ERROR: screen not set\n");
      return;
    }
    renderstereo(edge, args);
  } else if (strcmp(list[0], "spinc") == 0) {
    spincyclops(edge, args);
  } else if (strcmp(list[0], "spins") == 0) {
    spinstereo(edge, args);
  } else if (strcmp(list[0], "stdin") == 0) {
    if (in != stdin)
      fclose(in);
    in = stdin;
  } else if (strcmp(list[0], "file") == 0) {
    renderppm(list[1]);
  } else if (strcmp(list[0], "end") == 0) {
    quit = 1;
  } else {
    printf("invalid command: %s\n", list[0]);
  }
}

int main(int argc, char **argv) {
  edge = mat_construct(0, 4);
  tform = identity_mat();
  in = stdin;
  screen = 0;
  rendering_initialized = 0;
  if (argc > 1)
    in = fopen(argv[1], "r");
  char inbuf[MAX_LINE + 1];
  while (!quit) {
    quit = should_quit();
    fgets(inbuf, MAX_LINE, in);
    interpret(inbuf);
  }
  if (rendering_initialized)
    finish_live_render();
}
