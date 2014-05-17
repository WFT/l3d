#pragma once
#include <inttypes.h>

typedef struct bresenham_line {
  int x1, y1, z1, x2, y2, z2;
  int *x_points;
  int *y_points;
  double *z_points;
} BLine;

#define BLINE_DEFAULT = ((BLine){-1, -1, -1, -1, -1, -1, NULL, NULL, NULL});

// sets up the BLine using the endpoints already set
void bline_load(BLine *line);

// takes an array of six coordinates alternating x y z
void draw_triangle(int coors[9], uint32_t color);

// order points by x
void order_endpoints(BLine *line);

// discover all points using the bresenham_step
void find_points(BLine *line);

// generalized bresenham line algorithm will advance one step for each call
void bresenham_step(int *acc, int *major_counter, int *minor_counter, int major_delta, int minor_delta, int major_step, int minor_step);

// predicts how many points there will be
int point_count(BLine *line);
