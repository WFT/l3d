#include "transform.h"

Matrix * identity_mat() {
  Matrix *ret = mat_construct(4, 4);
  int i;
  for(i = 0; i < 4; i++)
    mat_set_cell(ret, i, i, 1);
  return ret;
}

Matrix * move_mat(double x, double y, double z) {
  Matrix *ret = identity_mat();
  mat_set_cell(ret, 3, 0, x);
  mat_set_cell(ret, 3, 1, y);
  mat_set_cell(ret, 3, 2, z);
  return ret;
}

Matrix * scale_mat(double x, double y, double z) {
  Matrix *ret = identity_mat();
  mat_set_cell(ret, 0, 0, x);
  mat_set_cell(ret, 1, 1, y);
  mat_set_cell(ret, 2, 2, z);
  return ret;
}

Matrix * rotate_x_mat(double rad) {
  Matrix *ret = identity_mat();
  mat_set_cell(ret, 1, 1, cos(rad));
  mat_set_cell(ret, 1, 2, sin(rad));
  mat_set_cell(ret, 2, 1, -sin(rad));
  mat_set_cell(ret, 2, 2, cos(rad));
  return ret;
}

Matrix * rotate_y_mat(double rad) {
  Matrix *ret = identity_mat();
  mat_set_cell(ret, 0, 0, cos(rad));
  mat_set_cell(ret, 2, 0, sin(rad));
  mat_set_cell(ret, 0, 2, -sin(rad));
  mat_set_cell(ret, 2, 2, cos(rad));
  return ret;
}

Matrix * rotate_z_mat(double rad) {
  Matrix *ret = identity_mat();
  mat_set_cell(ret, 0, 0, cos(rad));
  mat_set_cell(ret, 0, 1, sin(rad));
  mat_set_cell(ret, 1, 0, -sin(rad));
  mat_set_cell(ret, 1, 1, cos(rad));
  return ret;
}
