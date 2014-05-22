#pragma once
#include <SDL2/SDL.h>
#include "matrix.h"

// renders a single perspective -- access through render functions below
void renderperspective(Matrix *faces, double *eye, uint32_t color);

// rendering functions for screen
void rendercyclops(Matrix *faces, double *eye);
void renderstereo(Matrix *faces, double *eyes);
void renderparallel(Matrix *faces);
void spincyclops(Matrix *edge, double *eye, int del);
void spinstereo(Matrix *edge, double *eyes, int del);
