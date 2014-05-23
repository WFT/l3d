#include <stdlib.h>
#include <stdio.h>
#include "lines.h"
#include "display.h"
#include "options.h"

// order points by x
inline void order_endpoints(int *x1, int *y1, int *x2, int *y2) {
  if (*x1 > *x2) {
    int swap = *x1;
    *x1 = *x2;
    *x2 = swap;
    swap = *y1;
    *y1 = *y2;
    *y2 = swap;
  }
}

// generalized bresenham line algorithm will advance one step for each call
inline void bresenham_step(int *acc, int *major_counter, int *minor_counter, int major_delta, int minor_delta, int major_step, int minor_step) {
  *acc -= minor_delta;
  if (*acc < 0) {
    *minor_counter += minor_step;
    *acc += major_delta;
  }
  *major_counter += major_step;
}

// points should be ordered first
// predicts how many points there will be
inline int point_count(int x1, int y1, int x2, int y2)  {
  int dx = x2 - x1;
  int dy = y2 - y1;
  if (dy < 0)
    dy = -dy;
  return (dx > dy ? dx : dy) + 1;
}

inline void draw_horizontal(int x1, int x2, int y, uint32_t color) {
  int x = x1;
  int step = x1 < x2 ? 1 : -1;
  while (x <= x2) {
    setpix(x, y, color, 0);
    x+=step;
  }
}

// discover all points using the bresenham_step
// RETURNS: number of points actually found
inline int find_points(int x1, int y1, int x2, int y2,
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
      if (!pix_in_screen(x, y)) break;
      x_points[p] = x;
      y_points[p] = y;
      bresenham_step(&acc, &x, &y, dx, dy, 1, ystep);
      p++;
    }
  } else {
    int  acc = dy/2;
    char up = y1 < y2;
    while (up ? y <= y2 : y >= y2) {
      if (!pix_in_screen(x, y)) break;
      x_points[p] = x;
      y_points[p] = y;
      bresenham_step(&acc, &y, &x, dy, dx, ystep, 1);
      p++;
    }
  }
  return p;
}

// takes an array of six coordinates alternating x y z
void draw_triangle(int coors[6], uint32_t color) {
  int ax = coors[0];
  int ay = coors[1];
  int bx = coors[2];
  int by = coors[3];
  int cx = coors[4];
  int cy = coors[5];

  lock_surface();

#if DRAW_LINES
  int p;
  order_endpoints(&ax, &ay, &bx, &by);
  int ab_count = point_count(ax, ay, bx, by);
  int *ab_x_points = malloc(ab_count * sizeof(int));
  int *ab_y_points = malloc(ab_count * sizeof(int));
  ab_count = find_points(ax, ay, bx, by, 
	      ab_x_points, ab_y_points);
  for (p = 0; p < ab_count; p++)
    setpix(ab_x_points[p], ab_y_points[p], color, 0);

  bx = coors[2];
  by = coors[3];
  order_endpoints(&bx, &by, &cx, &cy);
  int bc_count = point_count(bx, by, cx, cy);
  int *bc_x_points = malloc(bc_count * sizeof(int));
  int *bc_y_points = malloc(bc_count * sizeof(int));
  bc_count = find_points(bx, by,  cx, cy, 
	      bc_x_points, bc_y_points);
  for (p = 0; p < bc_count; p++)
    setpix(bc_x_points[p], bc_y_points[p], color, 0);

  ax = coors[0];
  ay = coors[1];
  cx = coors[4];
  cy = coors[5];
  order_endpoints(&cx, &cy, &ax, &ay);
  int ca_count = point_count(cx, cy, ax, ay);
  int *ca_x_points = malloc(ca_count * sizeof(int));
  int *ca_y_points = malloc(ca_count * sizeof(int));
  ca_count = find_points(cx, cy, ax, ay,
	      ca_x_points, ca_y_points);
  for (p = 0; p < ca_count; p++)
   setpix(ca_x_points[p], ca_y_points[p], color, 0);
  
  int *abxs;
  int *abys;
  int abinc;
  if (ab_y_points[ab_count - 1] > ab_y_points[0]) {
    abxs = ab_x_points + ab_count - 1;
    abys = ab_y_points + ab_count - 1;
    abinc = -1;
  } else {
    abxs = ab_x_points;
    abys = ab_y_points;
    abinc = 1;
  }

  int *bcxs;
  int *bcys;
  int bcinc;
  if (bc_y_points[bc_count - 1] > bc_y_points[0]) {
    bcxs = bc_x_points + bc_count - 1;
    bcys = bc_y_points + bc_count - 1;
    bcinc = -1;
  } else {
    bcxs = bc_x_points;
    bcys = bc_y_points;
    bcinc = 1;
  }

  int *caxs;
  int *cays;
  int cainc;
  if (ca_y_points[ca_count - 1] > ca_y_points[0]) {
    caxs = ca_x_points + ca_count - 1;
    cays = ca_y_points + ca_count - 1;
    cainc = -1;
  } else {
    caxs = ca_x_points;
    cays = ca_y_points;
    cainc = 1;
  }

  ax = coors[0];
  ay = coors[1];
  bx = coors[2];
  by = coors[3];
  cx = coors[4];
  cy = coors[5];

  // find the mid y value
  int max_y = ay > by ? ay:by;
  max_y = cy > max_y ? cy:max_y;
  int min_y = ay < by ? ay:by;
  min_y = cy < min_y ? cy:min_y;
  int mid_y;
  if (ay != max_y && ay != min_y)
    mid_y = ay;
  else if (by != max_y && by != min_y)
    mid_y = by;
  else if (cy != max_y && cy != min_y)
    mid_y = cy;
  else 
    return;

  int *upper_segment_x = NULL;
  int *upper_segment_y = NULL;
  int upper_inc = 0;
  int *lower_segment_x = NULL;
  int *lower_segment_y = NULL;
  int lower_inc = 0;
  int *long_segment_x = NULL;
  int *long_segment_y = NULL;
  int long_inc = 0;

  if (mid_y == ay) {
    long_inc = bcinc;
    long_segment_x = bcxs;
    long_segment_y = bcys;
    if (max_y == by) {
      upper_inc = abinc;
      upper_segment_x = abxs;
      upper_segment_y = abys;

      lower_inc = cainc;
      lower_segment_x = caxs;
      lower_segment_y = cays;
    } else if (max_y == cy) {
      upper_inc = cainc;
      upper_segment_x = caxs;
      upper_segment_y = cays;

      lower_inc = abinc;
      lower_segment_x = abxs;
      lower_segment_y = abys;
    }
  } else if (mid_y == by) {
    long_inc = cainc;
    long_segment_x = caxs;
    long_segment_y = cays;
    if (max_y == ay) {
      upper_inc = abinc;
      upper_segment_x = abxs;
      upper_segment_y = abys;

      lower_inc = bcinc;
      lower_segment_x = bcxs;
      lower_segment_y = bcys;
    } else if (max_y == cy) {
      upper_inc = bcinc;
      upper_segment_x = bcxs;
      upper_segment_y = bcys;

      lower_inc = abinc;
      lower_segment_x = abxs;
      lower_segment_y = abys;
    }
  } else if (mid_y == cy) {
    long_inc = abinc;
    long_segment_x = abxs;
    long_segment_y = abys;
    if (max_y == ay) {
      upper_inc = cainc;
      upper_segment_x = caxs;
      upper_segment_y = cays;

      lower_inc = bcinc;
      lower_segment_x = bcxs;
      lower_segment_y = bcys;
    } else if (max_y == by) {
      upper_inc = bcinc;
      upper_segment_x = bcxs;
      upper_segment_y = bcys;
 
      lower_inc = cainc;
      lower_segment_x = caxs;
      lower_segment_y = cays;
    }
  }
  int longi = 0, shorti = 0, oldy = -1;
  do {
    draw_horizontal(upper_segment_x[shorti],
		    long_segment_x[longi],
		    upper_segment_y[shorti],
		    color);
    longi++;
    shorti++;
    
  } while (long_segment_y[longi] > mid_y);
    
  free(ab_x_points);
  free(ab_y_points);
  free(bc_x_points);
  free(bc_y_points);
  free(ca_x_points);
  free(ca_y_points);
#endif

  // if color is set to non-black colors
  // and DRAW_VERTICES is true, vertices will be drawn
#if DRAW_VERTICES
  if (pix_in_screen(ax, ay))
    setpix(ax, ay, color ? VERTICES_COLOR : color, 1);
  if (pix_in_screen(bx, by))
    setpix(bx, by, color ? VERTICES_COLOR : color, 1);
  if (pix_in_screen(cx, cy))
    setpix(cx, cy, color ? VERTICES_COLOR : color, 1);
#endif

  unlock_surface();

}
