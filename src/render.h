#pragma once
#include <SDL2/SDL.h>
#include "matrix.h"

// renders a single perspective -- access through render functions below
void renderperspective(Matrix *faces, double *eye, Matrix *colors);

// rendering functions for screen
void rendercyclops(Matrix *faces, double *eye, Matrix *colors);
//void renderstereo(Matrix *faces, double *eyes);
void spincyclops(Matrix *edge, double *eye, Matrix *colors, int del);
//void spinstereo(Matrix *edge, double *eyes, int del);
