typedef struct bresenham_line {
  int x1, y1, z1, x2, y2, z2;
  int acc_xy, acc_xz;
  int *x_points;
  int *y_points;
  int *z_points;
} BLine;

// order points by x
void order_endpoints(BLine *line);

// draw across
void draw_cross(BLine *l1, BLine *l2);

// discover all points from xyz1 to xyz2
void find_points(BLine *line);

// bresenham advance
void advance_line(BLine *line);
