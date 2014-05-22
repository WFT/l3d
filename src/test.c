#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "matrix.h"
#include "transform.h"
#include "objects.h"
#include "display.h"
#include "render.h"

void rand_point(double p[4]) {
  int i;
  for (i=0; i < 3; i++)
    p[i] = (((double)rand()/(double)RAND_MAX) * 20) - 10;
  p[3] = 1;
}

double rand_radian() {
  return ((double)rand()/(double)RAND_MAX) * 6.29;
}

void spin_test() {
  Matrix *faces = mat_construct(0, 4);
  double col[4] = {0, 0, 0, 1};
  clock_t t;
  double rad = rand_radian();
  unsigned long i;
  do {
    for (i=0; i < 3000; i++) {
      rand_point(col);
      mat_add_column(faces, col);
    }
    t = clock();
    apply_transform(rotate_z_mat(rad), &faces);
    t = clock() - t;
    printf("%d faces spun in %f seconds\n", faces->cols/3,((float)t)/CLOCKS_PER_SEC);
  } while (((float)t)/CLOCKS_PER_SEC < 1);
  mat_destruct(faces);
}

void render_spin_test() {
  Matrix *faces = mat_construct(0, 4);
  double col[4] = {0, 0, 0, 1};
  double eye[3] = {0, 0, 100};
  screen = malloc(4 * sizeof(double));
  screen[0] = -10;
  screen[1] = -10;
  screen[2] = 10;
  screen[3] = 10;
  if (init_live_render(600, 600))
    printf("live rendering setup failed... Exiting now.\n");
  clock_t t;
  double rad = rand_radian();
  unsigned long i;
  do {
    for (i=0; i < 3000; i++) {
      rand_point(col);
      mat_add_column(faces, col);
    }
    t = clock();
    apply_transform(rotate_y_mat(rad), &faces);
    rendercyclops(faces, eye);
    t = clock() - t;
    printf("%d faces spun in %f seconds\n", faces->cols/3, ((float)t)/CLOCKS_PER_SEC);
  } while (((float)t)/CLOCKS_PER_SEC < 1);
  renderppm("test.ppm");
  mat_destruct(faces);
}

int main(int argc, char **argv) {
  srand(time(NULL));
  render_spin_test();
  return 0;
}
