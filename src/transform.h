#pragma once
#include <math.h>
#include "matrix.h"

#define TO_RAD(deg) (deg * M_PI / 180)

Matrix *identity_mat();

Matrix *move_mat(double x, double y, double z);

Matrix *scale_mat(double x, double y, double z);

// takes some radians
Matrix *rotate_x_mat(double rad);
Matrix *rotate_y_mat(double rad);
Matrix *rotate_z_mat(double rad);

// the transform and old object will be free'd
void apply_transform(Matrix *transform, Matrix **obj);
