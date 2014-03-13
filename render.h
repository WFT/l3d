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

// renders a single perspective -- access through render functions below
void renderperspective(Matrix *edge, double *eye, uint32_t color);

// rendering functions for screen
void renderparallel(Matrix *edge);
void rendercyclops(Matrix *edge, double *eye);
void renderstereo(Matrix *edge, double *eyes);

void update_display();
void clear_screen();

void spin(Matrix *edge);

// call to clean up
void finish_live_render();

// 1 on quit event, else 0
char should_quit();
