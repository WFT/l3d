#include <inttypes.h>
#include "matrix.h"
#pragma once

char rendering_initialized;
double *screen;

// returns 0 on success, 1 on fail and prints error
int init_live_render(int w, int h);

// set and get pix from the surface
void setpix(int x, int y, uint32_t color, char lock);
uint32_t getpix(int x, int y, char lock);

//scale xy of world coordinates to fit screen
void map_coors(double *coors, int *mapped);

void dline(int *coors, uint32_t color);

// render screen to ppm
void renderppm(char *path);

// rendering functions for screen
void renderparallel(Matrix *edge);
void renderperspective(Matrix *edge);
void renderstereo(Matrix *edge);

void update_display();
void clear_screen();

// call to clean up
void finish_live_render();

// 1 on quit event, else 0
char should_quit();