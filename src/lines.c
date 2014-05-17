#include "lines.h"
#include "display.h"
#include <stdlib.h>
#include <stdio.h>

BLine *ab = &BLINE_DEFAULT;
BLine *bc = &BLINE_DEFAULT;
BLine *cd = &BLINE_DEFAULT;
BLine *da = &BLINE_DEFAULT;


void bline_load(BLine * line) {
  order_endpoints(line);
  int count = point_count(line);
  size_t b = count * sizeof(int);
  line->x_points = realloc(line->x_points, b);
  line->y_points = realloc(line->y_points, b);
  line->z_points = realloc(line->z_points, count * sizeof(double));
  find_points(line);
}

void draw_triangle(int coors[9], uint32_t color) {
  ab->x1 = coors[0];
  ab->y1 = coors[1];
  ab->z1 = coors[2];
  ab->x2 = coors[3];
  ab->y2 = coors[4];
  ab->z2 = coors[5];

  bc->x1 = coors[3];
  bc->y1 = coors[4];
  bc->z1 = coors[5];
  bc->x2 = coors[6];
  bc->y2 = coors[7];
  bc->z2 = coors[8];

  cd->x1 = coors[6];
  cd->y1 = coors[7];
  cd->z1 = coors[8];
  cd->x2 = coors[0];
  cd->y2 = coors[1];
  cd->z2 = coors[2];


  // find the mid y value
  int max_y = coors[1] > coors[4] ? coors[1]:coors[4];
  max_y = coors[7] > max_y ? coors[7]:max_y;
  int min_y = coors[1] < coors[4] ? coors[1]:coors[4];
  min_y = coors[7] < min_y ? coors[7]:min_y;
  int mid_y;
  if (coors[1] != max_y && coors[1] != min_y)
    mid_y = coors[1];
  else if (coors[4] != max_y && coors[4] != min_y)
    mid_y = coors[4];
  else if (coors[7] != max_y && coors[7] != min_y)
    mid_y = coors[7];

  bline_load(ab);
  bline_load(bc);
  bline_load(cd);

  lock_surface();

  int p, count = point_count(ab);
  for (p = 0; p < count; p++)
    setpix(ab->x_points[p], ab->y_points[p], color, 0);

  count = point_count(bc);
  for (p = 0; p < count; p++)
    setpix(bc->x_points[p], bc->y_points[p], color, 0);

  count = point_count(cd);
  for (p = 0; p < count; p++)
    setpix(ab->x_points[p], ab->y_points[p], color, 0);

  lock_surface();
}

void order_endpoints(BLine *line) {
  if (line->x1 > line->x2) {
    int swap = line->x1;
    line->x1 = line->x2;
    line->x2 = line->x1;
    swap = line->y1;
    line->y1 = line->y2;
    line->y2 = line->y1;
    swap = line->z1;
    line->z1 = line->z2;
    line->z2 = line->z1;
  }
}

void find_points(BLine *line) {
  int dx = line->x2 - line->x1;
  int dy = line->y2 > line->y1?line->y2 - line->y1:line->y1 - line->y2;
  int x = line->x1, y = line->y1;
  // y goes up if y1 is smaller than y2, else it goes down
  int ystep = line->y1 < line->y2 ? 1 : -1;
  int p = 0;
  if (dx > dy) {
    int acc  = dx/2;
    while (x < line->x2) {
      line->x_points[p] = x;
      line->y_points[p] = y;
      bresenham_step(&acc, &x, &y, dx, dy, 1, ystep);
      p++;
    }
  } else {
    int  acc = dy/2;
    char up = line->y1 < line->y2;
    while (up ? y <= line->y2 : y >= line->y2) {
      line->x_points[p] = x;
      line->y_points[p] = y;
      bresenham_step(&acc, &y, &x, dy, dx, ystep, 1);
      p++;
    }
  }
  // creates a rough approximation of evenly spaced z coordinates
  p = 0;
  double z = line->z1;
  int count = point_count(line);
  double zstep = ((double)line->z1 - (double)line->z2) / (double)count;
  while (p < count) {
    line->z_points[p] = z;
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

int point_count(BLine *line)  {
  int dx = line->x2 - line->x1;
  int dy = line->y2 - line->y1;
  //printf("%d points from (%d, %d) to (%d, %d)\n", dx > dy ? dx + 1 : dy + 1, line->x1, line->y1, line->x2, line->y2);
  return dx > dy ? dx + 1 : dy + 1;
}
