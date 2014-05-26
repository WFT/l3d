/*********************************************************/
/* This whole file is a massive hack			 */
/* Only the rest of the project is supposed to look good */
/*********************************************************/

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

char vary_keys[100][25];
double vary_values[100];
int lastvdex = -1;

char tform_keys[100][25];
Matrix *tform_mats[100];
int lastmdex = -1;

Matrix *tri;
Matrix *tform;
char quit = 0;
char sdl_initialized = 0;
FILE *in;

// will destroy the transform
void multiply_transform(Matrix *transform) {
  Matrix *temp = mat_multiply(tform, transform);
  mat_destruct(tform);
  tform = temp;
  mat_destruct(transform);
}

void interpret(char *l) {
  if (l[0] == '#')
    return;
  char **list = parse_split(l);
  if (list[0] == NULL)
    return;
  int argc = parse_numwords(list) - 1;
  double *args = calloc(argc, sizeof(double));
  int i;
  for (i = 0; i < argc; i++) {
    if (isalpha(list[i+1][0])
 	&& strcmp(list[0], "vary") != 0
	&& strcmp(list[0], "save") != 0
	&& strcmp(list[0], "restore") != 0
	&& strcmp(list[0], "file") != 0
	&& strcmp(list[0], "import") != 0
	&& strcmp(list[0], "files") != 0) {
      int j;
      char found = 0;
      for (j = lastvdex; j > -1; j--) {
	if (strcmp(vary_keys[j], list[i+1]) == 0) {
	  args[i] = vary_values[j];
	  //printf("('%s', %.2f) in %s (f%d)\n", list[i+1], vary_values[j], list[i], nowframe);
	  found = 1;
	  break;
	}
      }
      if (!found) {
	printf("%s not found in command %s (f%d) [checked %d vary_values]", list[i+1], list[0], nowframe, lastvdex+1);
	//for (j=0; j < lastvdex + 1; j++)
	//printf("(%s, %.2f), ", vary_keys[j], vary_values[j]);
	//printf("\n");
	free(args);
	return;
      }
    } else {
      args[i] = strtod(list[i+1], NULL);
    }
  }
  if (strcmp(list[0], "pixels") == 0) {
    if (rendering_initialized) {
      free(args);
      return;
    }
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
  } else if (strcmp(list[0], "tri") == 0) {
    printf("drawing some triangle\n");
    double col[4] = {0, 0, 0, 1};
    Matrix *triangle = mat_construct(0, 4);
    col[0] = args[0];
    col[1] = args[1];
    col[2] = args[2];
    mat_add_column(triangle, col);
    col[0] = args[3];
    col[1] = args[4];
    col[2] = args[5];
    mat_add_column(triangle, col); 
    col[0] = args[6];
    col[1] = args[7];
    col[2] = args[8];
    mat_add_column(triangle, col);
    Matrix *obj = mat_multiply(tform, triangle);
    mat_extend(tri, obj);
    mat_destruct(triangle);
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
  } else if (strcmp(list[0], "import") == 0) {
    //Matrix *obj = tri_file(list[1], args);
    //mat_extend(tri, obj);
    //mat_destruct(obj);
  } else if (strcmp(list[0], "save") == 0) {
    if (lastmdex >= 99) {
      printf("Up to 100 transform saves allowed. This is number %d\n", lastmdex + 1);
      free(args);
      return;
    }
    lastmdex++;
    //printf("Saving transforms with %s (%d)...\n", list[1], lastmdex);
    strcpy(tform_keys[lastmdex], list[1]);
    //printf("key copied.\n");
    tform_mats[lastmdex] = mat_construct(0, 4);
    mat_extend(tform_mats[lastmdex], tform);
    //printf("%s is:\n", list[1]);
  } else if (strcmp(list[0], "restore") == 0) {
    Matrix *p = NULL;
    int i;
    for (i = 0; i > -1; i--) {
      if (strcmp(list[1], tform_keys[i]) == 0) {
	p = tform_mats[i];
	break;
      }
    }
    if (p) {
      mat_destruct(tform);
      tform = mat_construct(0, 4);
      mat_extend(tform, p);
      //printf("transform is now:\n");
    } else {
      //printf("Saved transform '%s' not found in ", list[1]);
      // for (i=0;i>-1;i--)
      // 	printf("|('%s')|", tform_keys[i]);
      // printf("\n");
    }
  } else if (strcmp(list[0], "vary") == 0) {
    if (totalframes < 0) {
      printf("Initialize with frames command before varying.\n");
      free(args);
      return;
    } 
    if (lastvdex >= 99) {
      printf("Up to 100 variables allowed.\n");
      free(args);
      return;
    }
    lastvdex++;
    if (nowframe > args[3] && nowframe < args[4]) {
      //printf("varying with %s...\n", list[1]);
      strcpy(vary_keys[lastvdex], list[1]);
      //printf("key copied.\n");
      if (nowframe == startframe)
	vary_values[lastvdex] = args[1];
      else
	vary_values[lastvdex] = args[1] + ((nowframe - args[3]) * ((args[2] - args[1])/(args[4] - args[3])));
    }
  } else if (strcmp(list[0], "frames") == 0) {
    startframe = args[0];
    if (totalframes < 0)
      nowframe = args[0];
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
      free(args);
      return;
    }
    rendercyclops(tri, args);
  } else if (strcmp(list[0], "render-perspective-stereo") == 0 ||
	     strcmp(list[0], "rstereo") == 0) {
    if (!screen) {
      printf("ERROR: screen not set\n");
      free(args);
      return;
    }
    renderstereo(tri, args);
  } else if (strcmp(list[0], "spinc") == 0) {
    int delay = 13;
    if (argc > 3)
      delay = args[3];
    spincyclops(tri, args, delay);
  } else if (strcmp(list[0], "spins") == 0) {
    int delay = 13;
    if (argc > 6)
      delay = args[6];
    spinstereo(tri, args, delay);
  } else if (strcmp(list[0], "stdin") == 0) {
    if (in != stdin)
      fclose(in);
    in = stdin;
  } else if (strcmp(list[0], "autorc") == 0) {
    if (!autocyclops)
      autocyclops = malloc(3 * sizeof(double));
    memcpy(autocyclops, args, 3 * sizeof(double));
    if (autostereo != NULL) {
      free(autostereo);
      autostereo = NULL;
    }
  } else if (strcmp(list[0], "autors") == 0) {
    if (!autostereo)
      autostereo = malloc(6 * sizeof(double));
    memcpy(autostereo, args, 6 * sizeof(double));
    if (autocyclops != NULL) {
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
      free(args);
      return;
    }
    renderppm(fname);
    free(fname);
  } else if (strcmp(list[0], "file") == 0) {
    renderppm(list[1]);
  } else if (strcmp(list[0], "end") == 0) {
    if (nowframe < totalframes) {
      rewind(in);
      lastvdex = lastmdex = -1;
      printf("frame %d\n", nowframe);
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
  free(args);
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
    if (!fgets(inbuf, MAX_LINE, in))
      return 0;
    interpret(inbuf);
    if (autocyclops != NULL)
      rendercyclops(tri, autocyclops);
    else if (autostereo != NULL)
      renderstereo(tri, autostereo);
    if (nowframe == totalframes)
      quit = 1;
  }
  if (rendering_initialized)
    finish_live_display();
}
