#include "lines.h"
#include "display.h"
#include <stdlib.h>
#include <stdio.h>

char in_bounds(int x, int y, double bounds[4]) {
  return x > bounds[0] && y > bounds[1] && x < bounds[2] && y < bounds[3];
}

void draw_triangle(int coors[6], uint32_t color) {
  int ax = coors[0];
  int ay = coors[1];
  int bx = coors[2];
  int by = coors[3];
  int cx = coors[4];
  int cy = coors[5];

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
  order_endpoints(&ax, &ay, &bx, &by);

  int p, count = point_count(ax, ay, bx, by);
  int *x_points = malloc(count * sizeof(int));
  int *y_points = malloc(count * sizeof(int));

  count = find_points(ax, ay, bx, by, 
	      x_points, y_points);
  for (p = 0; p < count; p++)
    setpix(x_points[p], y_points[p], color, 0);

  bx = coors[2];
  by = coors[3];
  order_endpoints(&bx, &by, &cx, &cy);
  count = point_count(bx, by, cx, cy);
  free(x_points);
  free(y_points);
  x_points = malloc(count * sizeof(int));
  y_points = malloc(count * sizeof(int));
  count = find_points(bx, by,  cx, cy, 
	      x_points, y_points);
  for (p = 0; p < count; p++)
    setpix(x_points[p], y_points[p], color, 0);

  ax = coors[0];
  ay = coors[1];
  cx = coors[4];
  cy = coors[5];
  order_endpoints(&cx, &cy, &ax, &ay);
  count = point_count(cx, cy, ax, ay);
  free(x_points);
  free(y_points);
  x_points = malloc(count * sizeof(int));
  y_points = malloc(count * sizeof(int));
  count = find_points(cx, cy, ax, ay,
	      x_points, y_points);
  for (p = 0; p < count; p++)
    setpix(x_points[p], y_points[p], color, 0);

  unlock_surface();

  free(x_points);
  free(y_points);
}

void order_endpoints(int *x1, int *y1, int *x2, int *y2) {
  if (*x1 > *x2) {
    int swap = *x1;
    *x1 = *x2;
    *x2 = swap;
    swap = *y1;
    *y1 = *y2;
    *y2 = swap;
  }
  //printf("drawing (%d, %d) to (%d, %d)\n", *x1, *y1, *x2, *y2);
}

int find_points(int x1, int y1, int x2, int y2,
		 int *x_points, int *y_points) {
  int dx = x2 - x1;
  int dy = y2 > y1?y2 - y1:y1 - y2;
  int x = x1, y = y1;
  // y goes up if y1 is smaller than y2, else it goes down
  int ystep = y1 < y2 ? 1 : -1;
  int p = 0;
  if (dx > dy) {
    int acc  = dx/2;
    while (x < x2) {
      x_points[p] = x;
      y_points[p] = y;
      bresenham_step(&acc, &x, &y, dx, dy, 1, ystep);
      p++;
      if (!pix_in_screen(x, y)) break;
    }
  } else {
    int  acc = dy/2;
    char up = y1 < y2;
    while (up ? y <= y2 : y >= y2) {
      x_points[p] = x;
      y_points[p] = y;
      bresenham_step(&acc, &y, &x, dy, dx, ystep, 1);
      p++;
      if (!pix_in_screen(x, y)) break;
    }
  }
  return p;
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
  return (dx > dy ? dx : dy) + 1;
}
