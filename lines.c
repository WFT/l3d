#include "lines.h"

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

void draw_cross(BLine *l1, BLine *l2) {
  
}

void find_points(BLine *line) {
  
}

void advance_line(BLine *line) {
  
}
