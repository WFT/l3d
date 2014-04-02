#include "transform.h"
#include "objects.h"

Matrix *otransform(double *args);
void addtriangle(Matrix *mat, double *p1, double *p2, double *p3);
// s s s r r r m m m
Matrix *sphere_t(double *args) {
  int nVertices = 30;
  double lrad = 2 * M_PI / nVertices;
  Matrix *roty = rotate_y_mat(lrad);
  Matrix *sphere = mat_construct(0, 4);

  // gen original arc
  Matrix *arc = mat_construct(0, 4);
  double coors[4] = {0, 0, 0, 1};
  int i;
  for (i = 0; i < nVertices; i++) {
    coors[0] = cos(i * lrad);
    coors[1] = sin(i * lrad);
    mat_add_column(arc, coors);

    coors[0] = cos((i+1) * lrad);
    coors[1] = sin((i+1) * lrad);
    mat_add_column(arc, coors);
  }
  Matrix *arc2 = mat_multiply(roty, arc);
  Matrix *arcs = mat_construct(0, 4);
  double points[12];
  for (i = 0; i < 2 * nVertices - 1; i++) {
    mat_get_column(arc, i, points);
    mat_get_column(arc, i+1, points+4);
    mat_get_column(arc2, i+1, points+8);
    addtriangle(arcs, points, points+4, points+8); 
    mat_get_column(arc2, i+1, points+4);
    mat_get_column(arc2, i, points+8);
    addtriangle(arcs, points, points+4, points+8); 
  }
  mat_destruct(arc);
  mat_destruct(arc2);
  mat_destruct(roty);
  Matrix *rot2y = rotate_y_mat(2 * lrad);
  Matrix *tfrmd;
  for (i = 0; i < nVertices; i+=2) {
    tfrmd = mat_multiply(rot2y, arcs);
    mat_extend(sphere, tfrmd);
    mat_destruct(arcs);
    arcs = tfrmd;
  }
  Matrix *t  = otransform(args);
  Matrix *ret = mat_multiply(t, sphere);
  mat_destruct(sphere);
  mat_destruct(t);
  return ret;
}

Matrix *box_t(double *args) {
  Matrix *cube = mat_construct(0, 4);
  double tlf[4] = {-1, 1, 1, 1};	// top left front
  double tlb[4] = {-1, 1, -1, 1};	// top left back
  double trf[4] = {1, 1, 1, 1};		// top right front
  double trb[4] = {1, 1, -1, 1};	// top right back
  double blf[4] = {-1, -1, 1, 1};	// back left front
  double blb[4] = {-1, -1, -1, 1};	// back left back
  double brf[4] = {1, -1, 1, 1};	// back right front
  double brb[4] = {1, -1, -1, 1};	// back right back
  // top face
  addtriangle(cube, tlf, trb, tlb);
  addtriangle(cube, tlf, trf, trb);
  // bottom face
  addtriangle(cube, blf, brb, blb);
  addtriangle(cube, blf, brf, brb);
  // back face
  addtriangle(cube, blb, trb, tlb);
  addtriangle(cube, blb, brb, trb);
  // front face
  addtriangle(cube, blf, brf, trf);
  addtriangle(cube, blf, trf, tlf);
  // right face
  addtriangle(cube, brf, brb, trb);
  addtriangle(cube, brf, trb, trf);
  // left face
  addtriangle(cube, blf, blb, tlf);
  addtriangle(cube, tlf, blb, tlb);
  Matrix *t  = otransform(args);
  Matrix *ret = mat_multiply(t, cube);
  mat_destruct(cube);
  mat_destruct(t);
  return ret;
}

Matrix *otransform(double *args) {
  Matrix *t = scale_mat(args[0], args[1], args[2]);
  mat_multinmat(rotate_x_mat(TO_RAD(args[3])), t, t);
  mat_multinmat(rotate_y_mat(TO_RAD(args[4])), t, t);
  mat_multinmat(rotate_z_mat(TO_RAD(args[5])), t, t);
  mat_multinmat(move_mat(args[6], args[7], args[8]), t, t);
  return t;
}

void addtriangle(Matrix *mat, double *p1, double *p2, double *p3) {
  mat_add_column(mat, p1);
  mat_add_column(mat, p2);
  mat_add_column(mat, p3);
}
