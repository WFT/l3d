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

double *autocyclops = NULL;
double *autostereo = NULL;

int startframe = 0, nowframe = 0, totalframes = -1;

char keys[25][100];
double values[100];
int lastIndex = -1;

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
    if (isalpha(list[i+1][0]) 
	&& strcmp(list[0], "vary") != 0
	&& strcmp(list[0], "file") != 0
	&& strcmp(list[0], "files") != 0) {
      int j;
      char found = 0;
      for (j = lastIndex; j > -1; j--) {
	if (strcmp(keys[j], list[i+1]) == 0) {
	  args[i] = values[j];
	  printf("('%s', %.2f) in %s (f%d)\n", list[i+1], values[j], list[i], nowframe);
	  found = 1;
	  break;
	}
      }
      if (!found) {
	printf("%s not found in command %s (f%d) [checked %d values]", list[i+1], list[0], nowframe, lastIndex+1);
	for (j=0; j < lastIndex + 1; j++)
	  printf("(%s, %.2f), ", keys[j], values[j]);
	printf("\n");
	return;
      }
    } else {
      args[i] = strtod(list[i+1], NULL);
    }
  }
  if (strcmp(list[0], "pixels") == 0) {
    if (rendering_initialized) return;
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
  } else if (strcmp(list[0], "vary") == 0) {
    if (totalframes < 0) {
      printf("Initialize with frames command before varying.\n");
      return;
    } 
    if (lastIndex >= 99) {
      printf("Up to 100 variables allowed.\n");
      return;
    }
    lastIndex++;
    if (nowframe > args[3] && nowframe < args[4]) {
      strcpy(keys[lastIndex], list[1]);
      values[lastIndex] = (nowframe - args[3]) * ((args[2] - args[1])/(args[4] - args[3]));
    }
  } else if (strcmp(list[0], "frames") == 0) {
    startframe = args[0];
    totalframes = args[1] - args[0];
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
  } else if (strcmp(list[0], "autorc") == 0) {
    if (!autocyclops)
      autocyclops = malloc(3 * sizeof(double));
    memcpy(autocyclops, args, 3 * sizeof(double));
    if (autostereo) {
      free(autostereo);
      autostereo = NULL;
    }
  } else if (strcmp(list[0], "autors") == 0) {
    if (!autostereo)
      autostereo = malloc(6 * sizeof(double));
    memcpy(autostereo, args, 6 * sizeof(double));
    if (autocyclops) {
      free(autocyclops);
      autocyclops = NULL;
    }
  } else if (strcmp(list[0], "filein") == 0) {
    if (in != stdin)
      fclose(in);
    in = fopen(list[1], "r");
  } else if (strcmp(list[0], "files") == 0) {
    char *fname;
    if (asprintf(&fname, "%s%03d.ppm", list[1], nowframe) == -1) {
      printf("Who took all the memory?\n");
      return;
    }
    renderppm(fname);
    free(fname);
  } else if (strcmp(list[0], "file") == 0) {
    renderppm(list[1]);
  } else if (strcmp(list[0], "end") == 0) {
    if (nowframe < totalframes) {
      rewind(in);
      lastIndex = -1;
      nowframe++;
      mat_destruct(tri);
      mat_destruct(tform);
      tri = mat_construct(0, 4);
      tform = identity_mat();
    } else
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
    if (inbuf[0] == '\n') continue;
    interpret(inbuf);
    if (autocyclops)
      rendercyclops(tri, autocyclops);
    else if (autostereo)
      renderstereo(tri, autostereo);
    if (nowframe == totalframes)
      quit = 1;
  }
  if (rendering_initialized)
    finish_live_display();
}
