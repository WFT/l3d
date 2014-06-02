#pragma once
#include <inttypes.h>
#include "display.h"
// takes an array of six coordinates alternating x y z
void draw_triangle(KZ_Point a, KZ_Point b, KZ_Point c,
		   uint32_t color);
