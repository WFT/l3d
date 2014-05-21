#pragma once
#include <inttypes.h>

#define DRAW_LINES 1
#define DRAW_VERTICES 0
#define VERTICES_COLOR 0xFF0000

// takes an array of six coordinates alternating x y z
void draw_triangle(int coors[6], uint32_t color);

// order points by x
inline void order_endpoints(int *x1, int *y1, int *x2, int *y2);

// discover all points using the bresenham_step
// RETURNS: number of points actually found
inline int find_points(int x1, int y1, int x2, int y2,
		 int *x_points, int *y_points);

// generalized bresenham line algorithm will advance one step for each call
inline void bresenham_step(int *acc, int *major_counter, int *minor_counter, int major_delta, int minor_delta, int major_step, int minor_step);

inline void draw_horizontal(int x1, int x2, int y, uint32_t color);

// predicts how many points there will be
inline int point_count(int x1, int y1, int x2, int y2);
