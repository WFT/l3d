#include <stdlib.h>
#include <stdio.h>
#include "lines.h"
#include "display.h"
#include "options.h"

inline void order_endpoints(KZ_Point *p1, KZ_Point *p2) {
  if (p2->x > p1->x) {
    KZ_Point swap = *p1;
    *p1 = *p2;
    *p2 = swap;
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

// predicts how many points there will be
inline int point_count(KZ_Point p1, KZ_Point p2)  {
  KZ_Point greatP = p2;
  KZ_Point littleP = p1;
  order_endpoints(*littleP, *greatP);

  int dx = x2 - x1;
  int dy = y2 - y1;
  if (dy < 0)
    dy = -dy;
  return (dx > dy ? dx : dy) + 1;
}

inline void draw_horizontal(KZ_Point p1, KZ_Point p2, uint32_t color) {
  if (p1.x == p2.x) {
    setpix(p1.x, y, color, 0);
    return;
  }
  if (p1.x < p2.x && p1.x < 0)
    p1.x = 0;
  else if (p2.x <= p1.x && p2.x < 0)
    p2.x = 0;
  char has_drawn = 0;
  int x = p1.x;
  int xstep = p1.x < p2.x ? 1 : -1;
  double r = p1.x < p2.x ? p1.kr : p2.kr;
  double g = p1.x < p2.x ? p1.kg : p2.kg;
  double b = p1.x < p2.x ? p1.kb : p2.kb;
  double rstep = p1.x < p2.x ? (p2.kr - p1.kr) * (
  KZ_Point p;
  p.y = p1.y;
  while (p1.x < p2.x ? x <= p2.x : x >= p2.x) {
    if (pix_in_screen(x, y)) {
      p.x = x;
      setpix(x, y, color, 0);
      has_drawn = 1;
    } else if (has_drawn) {
      break;
    }
    x+=xstep;
  }
}

// discover all points using the bresenham_step
// RETURNS: number of points actually found
inline int find_points(KZ_Point p1, KZ_Point p2, int *points) {
  KZ_Point greatP = p2;
  KZ_Point littleP = p1;
  order_endpoints(*littleP, *greatP);

  int dx = greatP.x - p1.x;
  int dy = greatP.y > littlP.y?greatP.y - littlP.y:littlP.y - greatP.y;
  int x = p1.x, y = littlP.y;
  // y goes up if littlP.y is smaller than greatP.y, else it goes down
  int ystep = littlP.y < greatP.y ? 1 : -1;
  int p = 0;
  if (dx > dy) {
    int acc  = dx/2;
    while (x <= greatP.x) {
      x_points[p] = x;
      y_points[p] = y;
      bresenham_step(&acc, &x, &y, dx, dy, 1, ystep);
      p++;
    }
  } else {
    int  acc = dy/2;
    char up = littlP.y < greatP.y;
    while (up ? y <= greatP.y : y >= greatP.y) {
      x_points[p] = x;
      y_points[p] = y;
      bresenham_step(&acc, &y, &x, dy, dx, ystep, 1);
      p++;
    }
  }
  return p;
}

// takes an array of six coordinates alternating x y z
void draw_triangle(KZ_Point a, KZ_Point b, KZ_Point c,
		   uint32_t color) {

  lock_surface();
  
  
  int ab_count = point_count(a, b);
  KZ_Point *ab_points = malloc((ab_count + 1) * sizeof(KZ_Point));
  ab_count = find_points(a, b, ab_points);

  int bc_count = point_count(b, c);
  *bc_points = malloc((bc_count + 1) * sizeof(KZ_Point));
  bc_count = find_points(b, c, bc_points);

  int ca_count = point_count(c, a);
  *ca_points = malloc((ca_count + 1) * sizeof(KZ_Point));
  ca_count = find_points(c, a, ca_points);

#if DRAW_FILL

  // order arrays by y descending

  int *abs;
  int abinc;
  if (ab_y_points[ab_count - 1] > ab_y_points[0]) {
    abs = ab_points + ab_count - 1;
    abinc = -1;
  } else {
    abs = ab_points;
    abinc = 1;
  }

  int *bcs;
  int bcinc;
  if (bc_y_points[bc_count  - 1] > bc_y_points[0]) {
    bcs = bc_points + bc_count - 1;
    bcinc = -1;
  } else {
    bcs = bc_points;
    bcinc = 1;
  }

  int *cas;
  int cainc;
  if (ca_y_points[ca_count  - 1] > ca_y_points[0]) {
    cas = ca_points + ca_count - 1;
    cainc = -1;
  } else {
    cas = ca_points;
    cainc = 1;
  }

  // find the mid y value
  int max_y = a.y > b.y ? a.y:b.y;
  max_y = c.y > max_y ? c.y:max_y;
  int mid_y;
  if (max_y == a.y) {
    mid_y = b.y > c.y ? b.y : c.y;
  } else if (max_y == b.y) {
    mid_y = a.y > c.y ? a.y : c.y;
  } else if (max_y == c.y) {
    mid_y = b.y > a.y ? b.y : a.y;
  } else {
    printf("mid_y can't be found\n");
    return;
  }

  //categorize lines by upper, lower, long

  int *upper_segment = NULL;
  int upper_inc = 0;
  int upper_count = 0;
  int *lower_segment = NULL;
  int lower_inc = 0;
  int lower_count = 0;
  int *long_segment = NULL;
  int long_inc = 0;
  int long_count = 0;

  if (mid_y == ay) {
    long_inc = bcinc;
    long_segment = bcs;
    long_count = bc_count;
    if (max_y == by) {
      upper_inc = abinc;
      upper_segment = abs;
      upper_count = ab_count;

      lower_inc = cainc;
      lower_segment = cas;
      lower_count = ca_count;
    } else if (max_y == cy) {
      upper_inc = cainc;
      upper_segment = cas;
      upper_count = ca_count;

      lower_inc = abinc;
      lower_segment = abs;
      lower_count = ab_count;
    }
  } else if (mid_y == by) {
    long_inc = cainc;
    long_segment = cas;
    long_count = ca_count;
    if (max_y == ay) {
      upper_inc = abinc;
      upper_segment = abs;
      upper_count = ab_count;

      lower_inc = bcinc;
      lower_segment = bcs;
      lower_count = bc_count;
    } else if (max_y == cy) {
      upper_inc = bcinc;
      upper_segment = bcs;
      upper_count = bc_count;

      lower_inc = abinc;
      lower_segment = abs;
      lower_count = ab_count;
    }
  } else if (mid_y == cy) {
    long_inc = abinc;
    long_segment = abs;
    long_count = ab_count;
    if (max_y == ay) {
      upper_inc = cainc;
      upper_segment = cas;
      upper_count = ca_count;

      lower_inc = bcinc;
      lower_segment = bcs;
      lower_count = bc_count;
    } else if (max_y == by) {
      upper_inc = bcinc;
      upper_segment = bcs;
      upper_count = bc_count;

      lower_inc = cainc;
      lower_segment = cas;
      lower_count = ca_count;
    }
  }
  int longi = 0, shorti = 0;
  while (shorti * upper_inc < upper_count
         && upper_segment_y[shorti + upper_inc] == upper_segment_y[shorti]) {
    shorti += upper_inc;
  }  
  while (longi * long_inc < long_count
	 && (long_segment_y[longi + long_inc] == long_segment_y[longi]
	     || long_segment_y[longi] != upper_segment_y[shorti])) {
    longi += long_inc;
  }
  while (shorti * upper_inc < upper_count && longi * long_inc < long_count) {
    draw_horizontal(upper_segment_x[shorti], long_segment_x[longi],
                    upper_segment_y[shorti], color);
    do
      shorti += upper_inc;
    while (shorti * upper_inc < upper_count - 1
           && upper_segment_y[shorti + upper_inc] == upper_segment_y[shorti]);
    do
	longi += long_inc;
    while (longi * long_inc < long_count - 1
	   && long_segment_y[longi + long_inc] >= mid_y
	   && (long_segment_y[longi + long_inc] == long_segment_y[longi]
	       || (shorti * upper_inc < upper_count
		   && long_segment_y[longi] != upper_segment_y[shorti])
	       || (shorti * upper_inc >= upper_count
		   && long_segment_y[longi] > mid_y)));
  }
  shorti = 0;
  int stopper = long_segment_y[longi];
  while (shorti * lower_inc < lower_count - 1
 	 && (lower_segment_y[shorti] > stopper
	     || lower_segment_y[shorti + lower_inc] == lower_segment_y[shorti])) {
    shorti += lower_inc;
  }
  while (shorti * lower_inc < lower_count && longi * long_inc < long_count) {
    draw_horizontal(lower_segment_x[shorti], long_segment_x[longi],
		    lower_segment_y[shorti], color);
    do
      shorti += lower_inc;
    while (shorti * lower_inc < lower_count
	   && lower_segment_y[shorti + lower_inc] == lower_segment_y[shorti]);
    do
      longi += long_inc;
    while (longi * long_inc < long_count
	   && (long_segment_y[longi + long_inc] == long_segment_y[longi]
	       || (shorti * lower_inc < lower_count
		   && long_segment_y[longi] != lower_segment_y[shorti])
	       || (shorti * lower_inc >= lower_count
		   && long_segment_y[longi] > mid_y)));
  }
#endif

#if DRAW_EDGES
  int p;
  for (p = 0; p < ab_count; p++)
    setpix(ab_x_points[p], ab_y_points[p], EDGE_COLOR, 0);
  for (p = 0; p < bc_count; p++)
    setpix(bc_x_points[p], bc_y_points[p], EDGE_COLOR, 0);
  for (p = 0; p < ca_count; p++)
    setpix(ca_x_points[p], ca_y_points[p], EDGE_COLOR, 0);
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

  free(ab_x_points);
  free(ab_y_points);
  free(bc_x_points);
  free(bc_y_points);
  free(ca_x_points);
  free(ca_y_points);

  unlock_surface();
}
