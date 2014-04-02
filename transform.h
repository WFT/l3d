#pragma once
#include "matrix.h"

Matrix * identity_mat();

Matrix * move_mat(double x, double y, double z);

Matrix * scale_mat(double x, double y, double z);

// takes some radians
Matrix * rotate_x_mat(double rad);
Matrix * rotate_y_mat(double rad);
Matrix * rotate_z_mat(double rad);
