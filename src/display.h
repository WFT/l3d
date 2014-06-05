#include <inttypes.h>
#include "matrix.h"
#pragma once

// a struct to make lighting models + z-buffering possible
typedef struct kz_point {
  // x, y coordinates (screen)
  int x, y;
  // (distance from eye)^2
  double r;
  // k values for lighting model (one for each rgb channel)
  double kr, kg, kb;
} KZ_Point;

char rendering_initialized;
double *screen;

// returns 0 on success, 1 on fail and prints error
int init_live_render(int w, int h);

// setting a pixel for which this function returns 0 is undefined
// will probably result in a segfault
char pix_in_screen(int x, int y);

// set and get pix from the surface
void setpix(int x, int y, uint32_t color, char lock);
uint32_t getpix(int x, int y, char lock);

// inserts this point into the kz buffer if it isn't occluded
void consider_KZ_Point(KZ_Point p);

// draw the KZ_buffer to the screen
void flip_KZ_buffer();

// lock the surface prior to drawing
// if setting many pixels, lock first and tell setpix()
// not to lock by passing 0 as the last parameter
void lock_surface();

// always manually unlock after manually locking
// if setpix and getpix are locking (passing 1 as last parameter)
// then they also unlock, automatically
void unlock_surface();

// returns a 32 bit int representing an rgb color in the surface format
uint32_t rgb(int r, int g, int b);

//scale xy of world coordinates to fit screen
void map_coors(double *x, double *y);

void dline(int *coors, uint32_t color);

// render screen to ppm
void renderppm(char *path);

void update_display();
void clear_screen();
// also clears the kz_buf
void clear_pixel_buffer();

void mixcolors(char b);

// call to clean up
void finish_live_display();

// 1 on quit event, else 0
char should_quit();
