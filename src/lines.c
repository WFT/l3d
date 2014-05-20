#include "lines.h"
#include "display.h"
#include <stdlib.h>
#include <stdio.h>

void draw_triangle(int coors[9], uint32_t color) {
  int ab_x1 = coors[0];
  int ab_y1 = coors[1];
  int ab_z1 = coors[2];
  int ab_x2 = coors[3];
  int ab_y2 = coors[4];
  int ab_z2 = coors[5];

	printf("A(%d, %d) to B(%d, %d)\n", ab_x1, ab_y1, ab_x2, ab_y2);
  order_endpoints(&ab_x1, &ab_y1, &ab_z1, &ab_x1, &ab_y2, &ab_z2);
printf("A(%d, %d) to B(%d, %d) post order\n", ab_x1, ab_y1, ab_x2, ab_y2);

  int bc_x1 = coors[3];
  int bc_y1 = coors[4];
  int bc_z1 = coors[5];
  int bc_x2 = coors[6];
  int bc_y2 = coors[7];
  int bc_z2 = coors[8];

  order_endpoints(&bc_x1, &bc_y1, &bc_z1, &bc_x1, &bc_y2, &bc_z2);

  int ca_x1 = coors[6];
  int ca_y1 = coors[7];
  int ca_z1 = coors[8];
  int ca_x2 = coors[0];
  int ca_y2 = coors[1];
  int ca_z2 = coors[2];

  order_endpoints(&ca_x1, &ca_y1, &ca_z1, &ca_x1, &ca_y2, &ca_z2);

  // find the mid y value
  /* int max_y = coors[1] > coors[4] ? coors[1]:coors[4]; */
  /* max_y = coors[7] > max_y ? coors[7]:max_y; */
  /* int min_y = coors[1] < coors[4] ? coors[1]:coors[4]; */
  /* min_y = coors[7] < min_y ? coors[7]:min_y; */
  /* int mid_y; */
  /* if (coors[1] != max_y && coors[1] != min_y) */
  /*   mid_y = coors[1]; */
  /* else if (coors[4] != max_y && coors[4] != min_y) */
  /*   mid_y = coors[4]; */
  /* else if (coors[7] != max_y && coors[7] != min_y) */
  /*   mid_y = coors[7]; */


  lock_surface();

  int p, count = point_count(ab_x1, ab_y1, ab_x2, ab_y2);
  //printf("%d points (%lu B malloc'd) (%d, %d) to (%d, %d)\n", count, count * sizeof(int), ab_x1, ab_y1, ab_x2, ab_y2);
  int *x_points = malloc(count * sizeof(int));
  int *y_points = malloc(count * sizeof(int));
  int *z_points = malloc(count * sizeof(int));
  find_points(ab_x1, ab_y1, ab_z1, ab_x2, ab_y2, ab_z2,
	      x_points, y_points, z_points);
  for (p = 0; p < count; p++)
    setpix(x_points[p], y_points[p], color, 0);

  count = point_count(bc_x1, bc_y1, bc_x2, bc_y2);
  //printf("%d points (%lu B malloc'd) (%d, %d) to (%d, %d)\n", count, count * sizeof(int), bc_x1, bc_y1, bc_x2, bc_y2);
  x_points = realloc(x_points, count * sizeof(int));
  y_points = realloc(y_points, count * sizeof(int));
  z_points = realloc(z_points, count * sizeof(int));
  find_points(bc_x1, bc_y1, bc_z1, bc_x2, bc_y2, bc_z2,
	      x_points, y_points, z_points);
  for (p = 0; p < count; p++)
    setpix(x_points[p], y_points[p], color, 0);

  count = point_count(ca_x1, ca_y1, ca_x2, ca_y2);
  //printf("%d points (%lu B malloc'd) (%d, %d) to (%d, %d)\n", count, count * sizeof(int), ca_x1, ca_y1, ca_x2, ca_y2);
  x_points = realloc(x_points, count * sizeof(int));
  y_points = realloc(y_points, count * sizeof(int));
  z_points = realloc(z_points, count * sizeof(int));
  find_points(ca_x1, ca_y1, ca_z1, ca_x2, ca_y2, ca_z2,
	      x_points, y_points, z_points);
  for (p = 0; p < count; p++)
    setpix(x_points[p], y_points[p], color, 0);
  free(x_points);
  free(y_points);
  free(z_points);

  unlock_surface();
}

void order_endpoints(int *x1, int *y1, int *z1,
		     int *x2, int *y2, int *z2) {
  if (*x1 > *x2) {
    int swap = *x1;
    *x1 = *x2;
    *x2 = swap;
    swap = *y1;
    *y1 = *y2;
    *y2 = swap;
    swap = *z1;
    *z1 = *z2;
    *z2 = swap;
  }
}

void find_points(int x1, int y1, int z1, int x2, int y2, int z2,
		 int *x_points, int *y_points, int *z_points) {
  int dx = x2 - x1;
  int dy = y2 > y1?y2 - y1:y1 - y2;
  int x = x1, y = y1;
  // y goes up if y1 is smaller than y2, else it goes down
  int ystep = y1 < y2 ? 1 : -1;
  int p = 0;
  if (dx > dy) {
    int acc  = dx/2;
    while (x < x2) {
      if (p >= point_count(x1, y1, x2, y2))
	printf("count failure\n");
      x_points[p] = x;
      y_points[p] = y;
      bresenham_step(&acc, &x, &y, dx, dy, 1, ystep);
      p++;
    }
  } else {
    int  acc = dy/2;
    char up = y1 < y2;
    while (up ? y <= y2 : y >= y2) {
      x_points[p] = x;
      y_points[p] = y;
      bresenham_step(&acc, &y, &x, dy, dx, ystep, 1);
      p++;
    }
  }
  // creates a rough approximation of evenly spaced z coordinates
  p = 0;
  double z = z1;
  int count = point_count(x1, y1, x2, y2);
  double zstep = ((double)z1 - (double)z2) / (double)count;
  while (p < count) {
    z_points[p] = z;
    z += zstep;
    p++;
  }
}

void bresenham_step(int *acc, int *major_counter, int *minor_counter, int major_delta, int minor_delta, int major_step, int minor_step) {
  *acc -= minor_delta;
  if (*acc < 0) {
    *minor_counter += minor_step;
    *acc += major_delta;
  }
  *major_counter += major_step;
}

void draw_horizontals(int x1, int x2, int y, uint32_t color) {
  int x = x1;
  while (x <= x2) {
    setpix(x, y, color, 0);
    x++;
  }
}

// points should be ordered first
int point_count(int x1, int y1, int x2, int y2)  {
  int dx = x2 - x1;
  int dy = y2 - y1;
  if (dy < 0)
    dy = -dy;
  return dx > dy ? dx + 1 : dy + 1;
}
