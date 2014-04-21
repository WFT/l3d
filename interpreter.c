 #include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "display.h"
#include "render.h"
#include "matrix.h"
#include "parse_util.h"
#include "transform.h"
#include "objects.h"

#define TO_RAD(deg) (deg * M_PI / 180)

Matrix *tri;
Matrix *tform;
char quit = 0;
char sdl_initialized = 0;
FILE *in;

void multiply_transform(Matrix *transform) {
  //mat_multinmat(transform, tform, tform);
  Matrix *temp = mat_multiply(transform, tform);
  mat_destruct(tform);
  tform = temp;
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
  } else if (strcmp(list[0], "sphere-t") == 0) {
    Matrix *sphere = sphere_t(args);
    Matrix *obj = mat_multiply(tform, sphere);
    mat_extend(tri, obj);
    mat_destruct(sphere);
    mat_destruct(obj);
  } else if (strcmp(list[0], "box-t") == 0) {
    Matrix *cube = box_t(args);
    Matrix *obj = mat_multiply(tform, cube);
    mat_extend(tri, obj);
    mat_destruct(cube);
    mat_destruct(obj);
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
  } else if (strcmp(list[0], "clear-triangles") == 0) {
    mat_destruct(tri);
    tri = mat_construct(0, 4);
  } else if (strcmp(list[0], "clear-pixels") == 0) {
    clear_screen();
  } else if (strcmp(list[0], "ptri") == 0) {
    pmat(tri);
  } else if (strcmp(list[0], "render-perspective-cyclops") == 0 ||
	     strcmp(list[0], "rcyclops") == 0) {
    if (!screen) {
      printf("ERROR: screen not set\n");
      return;
    }
    rendercyclops(tri, args);
  } else if (strcmp(list[0], "render-perspective-stereo") == 0 ||
	     strcmp(list[0], "rstereo") == 0) {
    if (!screen) {
      printf("ERROR: screen not set\n");
      return;
    }
    renderstereo(tri, args);
  } else if (strcmp(list[0], "render-parallel") == 0 ||
	     strcmp(list[0], "rparallel") == 0) {
    if (!screen) {
      printf("ERROR: screen not set\n");
      return;
    }
    renderparallel(tri);
  } else if (strcmp(list[0], "spinc") == 0) {
    spincyclops(tri, args);
  } else if (strcmp(list[0], "spins") == 0) {
    spinstereo(tri, args);
  } else if (strcmp(list[0], "stdin") == 0) {
    if (in != stdin)
      fclose(in);
    in = stdin;
  } else if (strcmp(list[0], "filein") == 0) {
    if (in != stdin)
      fclose(in);
    in = fopen(list[1], "r");
  } else if (strcmp(list[0], "file") == 0) {
    renderppm(list[1]);
  } else if (strcmp(list[0], "end") == 0) {
    quit = 1;
  } else {
    printf("invalid command: %s\n", list[0]);
  }
}

int main(int argc, char **argv) {
  tri = mat_construct(0, 4);
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
    finish_live_display();
}
