#include <stdlib.h>
#include <stdio.h>
#include "lines.h"
#include "display.h"
#include "render.h"
#include "options.h"

inline void order_endpoints(KZ_Point *p1, KZ_Point *p2) {
  if (p2->x < p1->x) {
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
  order_endpoints(&littleP, &greatP);

  int dx = greatP.x - littleP.x;
  int dy = greatP.y - littleP.y;
  if (dy < 0)
    dy = -dy;
  return (dx > dy ? dx : dy) + 1;
}

inline void draw_horizontal(KZ_Point p1, KZ_Point p2) {
  if (p1.x == p2.x) {
    consider_KZ_Point(p1);
    return;
  }
  KZ_Point littleP = p1, greatP = p2;
  order_endpoints(&littleP, &greatP);
  if (littleP.x < 0)
    littleP.x = 0;
  char has_drawn = 0;
  double dx = greatP.x - littleP.x;
  double radstep = (greatP.r - littleP.r) / dx;
  double rstep = (greatP.kr - littleP.kr) / dx;
  double gstep = (greatP.kg - littleP.kg) / dx;
  double bstep = (greatP.kb - littleP.kb) / dx;
  KZ_Point p = littleP;
  while (p.x <= greatP.x) {
    if (pix_in_screen(p.x, p.y)) {
      consider_KZ_Point(p);
      has_drawn = 1;
     } else if (has_drawn) {
      break;
    }
    p.x++;
    p.kr += rstep;
    p.kg += gstep;
    p.kb += bstep;
    p.r += radstep;
    p.ared  = p.kr * ambient_red;
    p.agreen = p.kg * ambient_green;
    p.ablue = p.kb * ambient_blue;
  }
}

// discover all points using the bresenham_step
// RETURNS: number of points actually found
inline int find_points(KZ_Point p1, KZ_Point p2, KZ_Point *points) {
  KZ_Point greatP = p2;
  KZ_Point littleP = p1;
  order_endpoints(&littleP, &greatP);
  int dx = greatP.x - littleP.x;
  int dy = greatP.y > littleP.y ? greatP.y - littleP.y : littleP.y - greatP.y;
  // y goes up if littleP.y is smaller than greatP.y, else it goes down
  int ystep = littleP.y < greatP.y ? 1 : -1;
  KZ_Point p = littleP;

  double radstep = (greatP.r - littleP.r) / (double)dx;
  double rstep = (greatP.kr - littleP.kr) / (double)dx;
  double gstep = (greatP.kg - littleP.kg) / (double)dx;
  double bstep = (greatP.kb - littleP.kb) / (double)dx;
  if (greatP.x == littleP.x)
    radstep = rstep = gstep = bstep = 0;

  int i = 0;
  if (dx > dy) {
    int acc  = dx/2;
    while (p.x <= greatP.x) {
      points[i] = p;
      bresenham_step(&acc, &p.x, &p.y, dx, dy, 1, ystep);
      p.kr += rstep;
      p.kg += gstep;
      p.kb += bstep;
      p.r += radstep;
      p.ared = p.kr * ambient_red;
      p.agreen = p.kg * ambient_green;
      p.ablue = p.kb * ambient_blue;
      i++;
    }
  } else {
    int  acc = dy/2;
    char up = littleP.y < greatP.y;
    while (up ? p.y <= greatP.y : p.y >= greatP.y) {
      points[i] = p;
      bresenham_step(&acc, &p.y, &p.x, dy, dx, ystep, 1);
      p.kr += rstep;
      p.kg += gstep;
      p.kb += bstep;
      p.r += radstep;
      p.ared = p.kr * ambient_red;
      p.agreen = p.kg * ambient_green;
      p.ablue = p.kb * ambient_blue;
      i++;
    }
  }
  return i;
}

// takes an array of six coordinates alternating x y z
void draw_triangle(KZ_Point a, KZ_Point b, KZ_Point c) {
  int ab_count = point_count(a, b);
  KZ_Point *ab_points = malloc((ab_count + 1) * sizeof(KZ_Point));
  if (ab_points == NULL)
    printf("ab broking\n");
  ab_count = find_points(a, b, ab_points);

  int bc_count = point_count(b, c);
  KZ_Point *bc_points = malloc((bc_count + 1) * sizeof(KZ_Point));
  if (bc_points == NULL)
    printf("bc broking\n");
  bc_count = find_points(b, c, bc_points);

  int ca_count = point_count(c, a);
  KZ_Point *ca_points = malloc((ca_count + 1) * sizeof(KZ_Point));
  if (ca_points == NULL)
    printf("ca broking\n");
  ca_count = find_points(c, a, ca_points);

#if DRAW_FILL

  // order arrays by y descending

  KZ_Point *abs;
  int abinc;
  if (ab_points[ab_count - 1].y > ab_points[0].y) {
    abs = ab_points + ab_count - 1;
    abinc = -1;
  } else {
    abs = ab_points;
    abinc = 1;
  }

  KZ_Point *bcs;
  int bcinc;
  if (bc_points[bc_count  - 1].y > bc_points[0].y) {
    bcs = bc_points + bc_count - 1;
    bcinc = -1;
  } else {
    bcs = bc_points;
    bcinc = 1;
  }

  KZ_Point *cas;
  int cainc;
  if (ca_points[ca_count  - 1].y > ca_points[0].y) {
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

  KZ_Point *upper_segment = NULL;
  int upper_inc = 0;
  int upper_count = 0;
  KZ_Point *lower_segment = NULL;
  int lower_inc = 0;
  int lower_count = 0;
  KZ_Point *long_segment = NULL;
  int long_inc = 0;
  int long_count = 0;

  if (mid_y == a.y) {
    long_inc = bcinc;
    long_segment = bcs;
    long_count = bc_count;
    if (max_y == b.y) {
      upper_inc = abinc;
      upper_segment = abs;
      upper_count = ab_count;

      lower_inc = cainc;
      lower_segment = cas;
      lower_count = ca_count;
    } else if (max_y == c.y) {
      upper_inc = cainc;
      upper_segment = cas;
      upper_count = ca_count;

      lower_inc = abinc;
      lower_segment = abs;
      lower_count = ab_count;
    }
  } else if (mid_y == b.y) {
    long_inc = cainc;
    long_segment = cas;
    long_count = ca_count;
    if (max_y == a.y) {
      upper_inc = abinc;
      upper_segment = abs;
      upper_count = ab_count;

      lower_inc = bcinc;
      lower_segment = bcs;
      lower_count = bc_count;
    } else if (max_y == c.y) {
      upper_inc = bcinc;
      upper_segment = bcs;
      upper_count = bc_count;

      lower_inc = abinc;
      lower_segment = abs;
      lower_count = ab_count;
    }
  } else if (mid_y == c.y) {
    long_inc = abinc;
    long_segment = abs;
    long_count = ab_count;
    if (max_y == a.y) {
      upper_inc = cainc;
      upper_segment = cas;
      upper_count = ca_count;

      lower_inc = bcinc;
      lower_segment = bcs;
      lower_count = bc_count;
    } else if (max_y == b.y) {
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
         && upper_segment[shorti + upper_inc].y == upper_segment[shorti].y) {
    shorti += upper_inc;
  }  
  while (longi * long_inc < long_count
	 && (long_segment[longi + long_inc].y == long_segment[longi].y
	     || long_segment[longi].y != upper_segment[shorti].y)) {
    longi += long_inc;
  }
  while (shorti * upper_inc < upper_count && longi * long_inc < long_count) {
    draw_horizontal(upper_segment[shorti], long_segment[longi]);
    do
      shorti += upper_inc;
    while (shorti * upper_inc < upper_count - 1
           && upper_segment[shorti + upper_inc].y == upper_segment[shorti].y);
    do
	longi += long_inc;
    while (longi * long_inc < long_count - 1
	   && long_segment[longi + long_inc].y >= mid_y
	   && (long_segment[longi + long_inc].y == long_segment[longi].y
	       || (shorti * upper_inc < upper_count
		   && long_segment[longi].y != upper_segment[shorti].y)
	       || (shorti * upper_inc >= upper_count
		   && long_segment[longi].y > mid_y)));
  }
  shorti = 0;
  int stopper = long_segment[longi].y;
  while (shorti * lower_inc < lower_count - 1
 	 && (lower_segment[shorti].y > stopper
	     || lower_segment[shorti + lower_inc].y == lower_segment[shorti].y)) {
    shorti += lower_inc;
  }
  while (shorti * lower_inc < lower_count && longi * long_inc < long_count) {
    draw_horizontal(lower_segment[shorti], long_segment[longi]);
    do
      shorti += lower_inc;
    while (shorti * lower_inc < lower_count
	   && lower_segment[shorti + lower_inc].y == lower_segment[shorti].y);
    do
      longi += long_inc;
    while (longi * long_inc < long_count
	   && (long_segment[longi + long_inc].y == long_segment[longi].y
	       || (shorti * lower_inc < lower_count
		   && long_segment[longi].y != lower_segment[shorti].y)
	       || (shorti * lower_inc >= lower_count
		   && long_segment[longi].y > mid_y)));
  }
#endif

  
#if DRAW_EDGES
  int p;
  for (p = 0; p < ab_count; p++) {
    ab_points[p].r = -1;
    ab_points[p].kr = EDGE_RED;
    ab_points[p].kg = EDGE_GREEN;
    ab_points[p].kb = EDGE_BLUE;
    consider_KZ_Point(ab_points[p]);
  }
  for (p = 0; p < bc_count; p++) {
    bc_points[p].r = -1;
    bc_points[p].kr = EDGE_RED;
    bc_points[p].kg = EDGE_GREEN;
    bc_points[p].kb = EDGE_BLUE;
    consider_KZ_Point(bc_points[p]);
  }
  for (p = 0; p < ca_count; p++) {
    ca_points[p].r = -1;
    ca_points[p].kr = EDGE_RED;
    ca_points[p].kg = EDGE_GREEN;
    ca_points[p].kb = EDGE_BLUE;
    consider_KZ_Point(ca_points[p]);
  }
#endif
  
#if DRAW_VERTICES
  a.r = b.r = c.r = -1;
  a.kr = b.kr = c.kr = VERTICES_RED;
  a.kg = b.kg = c.kg = VERTICES_BLUE;
  a.kb = b.kb = c.kb = VERTICES_GREEN;
  if (pix_in_screen(a.x, a.y))
    consider_KZ_Point(a);
  if (pix_in_screen(b.x, b.y))
    consider_KZ_Point(b);
  if (pix_in_screen(c.x, c.y))
    consider_KZ_Point(c);
#endif
  

  free(ab_points);
  free(bc_points);
  free(ca_points);
}
