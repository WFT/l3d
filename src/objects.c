#include <stdlib.h>
#include <stdio.h>
#include "transform.h"
#include "parse_util.h"
#include "objects.h"

Matrix *otransform(double *args);
void addtriangle(Matrix *mat, double *p1, double *p2, double *p3);


// We'll fix this later -- for now we've got to work on docs
/*
Matrix *tri_file(char *fname, double *args) {
  FILE *f = fopen(fname, "r");
  char linein[MAX_LINE];
  linein[0] = '#';
  while (linein[0] == '#') {
    if (!fgets(linein, 99, f)) {
      printf("File read failed.");
      return;
    }
  }
  int ctri = atoi(linein);
  Matrix *obj = mat_construct(0, 4);
  double col[4];
  int argc, i;
  char **list;
  printf("adding %d triangles from file %s.\n", ctri, fname);
  while (fgets(linein, MAX_LINE-1, f) && ctri > 0) {
    if (linein[0] == '#') {
      return;
    }
    list = parse_split(linein);
    argc = parse_numwords(list) - 1;
    for (i = 0; i < argc && list[i]; i++) {
      printf("gothere%s\n", list[i]);
      col[i] = strtod(list[i], NULL);
    }
    mat_add_column(obj, col);
    ctri--;
  }
  Matrix *t  = otransform(args);
  Matrix *ret = mat_multiply(t, obj);
  return ret;
}
*/

Matrix *sphere_t(double *args) {
  int nVertices = 15;
  double lrad = (M_PI)/ nVertices;
  Matrix *roty = rotate_y_mat(lrad);
  Matrix *sphere = mat_construct(0, 4);
  
  Matrix *arc = mat_construct(0,4);
  double coors[4] = {0, 0, 0, 1};
  int i;
  // generate one arc of points, along z = 0
  for (i = 0; i < nVertices; i++) {
    coors[0] = sin(i * lrad);
    coors[1] = cos(i * lrad);
    mat_add_column(arc, coors);
  }
  Matrix *arcp = mat_multiply(roty, arc);
  Matrix *weave = mat_construct(0,4);
  double col[4];
  // turn the previous arcs into a series of triangles
  for (i = 0; i < nVertices - 1; i++) {
    mat_get_column(arc, i, col);
    mat_add_column(weave, col);
    mat_get_column(arc, i+1, col);
    mat_add_column(weave, col);
    mat_get_column(arcp, i, col);
    mat_add_column(weave, col);
    //arcp, i
    mat_add_column(weave, col);
    mat_get_column(arc, i+1, col);
    mat_add_column(weave, col);
    mat_get_column(arcp, i+1, col);
    mat_add_column(weave, col);
  }
  // end triangles!
  double end[4] = {0, -1, 0, 1};
  mat_get_column(arc, nVertices - 1, col);
  mat_add_column(weave, col);
  mat_add_column(weave, end);
  mat_get_column(arcp, nVertices - 1, col);
  mat_add_column(weave, col);
  mat_add_column(weave, col);
  mat_add_column(weave, end);
  mat_add_column(weave, end);
  // spin the woven matrix and zip it up
  Matrix *tfrmd;
  for (i = 0; i < nVertices * 2; i++) {
    tfrmd = mat_multiply(roty, weave);
    mat_extend(sphere, weave);
    mat_destruct(weave);
    weave = tfrmd;
  }
  Matrix *t  = otransform(args);
  Matrix *ret = mat_multiply(t, sphere);
  mat_destruct(roty);
  mat_destruct(arcp);
  mat_destruct(arc);
  mat_destruct(weave);
  mat_destruct(sphere);
  mat_destruct(t);
  return ret;
}

Matrix *box_t(double *args) {
  Matrix *cube = mat_construct(0, 4);
  double tlf[4] = {-.5, .5, .5, 1};	// top left front
  double tlb[4] = {-.5, .5, -.5, 1};	// top left back
  double trf[4] = {.5, .5, .5, 1};	// top right front
  double trb[4] = {.5, .5, -.5, 1};	// top right back
  double blf[4] = {-.5, -.5, .5, 1};	// back left front
  double blb[4] = {-.5, -.5, -.5, 1};	// back left back
  double brf[4] = {.5, -.5, .5, 1};	// back right front
  double brb[4] = {.5, -.5, -.5, 1};	// back right back
  // top face
  addtriangle(cube, tlf, trb, tlb);
  addtriangle(cube, tlf, trf, trb);
  // bottom face
  addtriangle(cube, blf, brb, brf);
  addtriangle(cube, blf, blb, brb);
  // back face
  addtriangle(cube, blb, trb, brb);
  addtriangle(cube, blb, tlb, trb);
  // front face
  addtriangle(cube, blf, brf, trf);
  addtriangle(cube, blf, trf, tlf);
  // right face
  addtriangle(cube, brf, brb, trb);
  addtriangle(cube, brf, trb, trf);
  // left face
  addtriangle(cube, blf, tlf, blb);
  addtriangle(cube, blb, tlf, tlb);
  Matrix *t  = otransform(args);
  Matrix *ret = mat_multiply(t, cube);
  mat_destruct(cube);
  mat_destruct(t);
  return ret;
}

// turn the arguments into a transformation matrix
// using the standard Sx Sy Sz Rx Ry Rz Mx My Mz
Matrix *otransform(double *args) {
  Matrix *t = scale_mat(args[0], args[1], args[2]);
  Matrix *temp;
  temp = mat_multiply(rotate_x_mat(TO_RAD(args[3])), t);
  mat_destruct(t);
  t = temp;
  temp = mat_multiply(rotate_y_mat(TO_RAD(args[4])), t);
  mat_destruct(t);
  t = temp;
  temp = mat_multiply(rotate_z_mat(TO_RAD(args[5])), t);
  mat_destruct(t);
  t = temp;
  temp = mat_multiply(move_mat(args[6], args[7], args[8]), t);
  mat_destruct(t);
  return temp;
}

// add three points to a matrix
void addtriangle(Matrix *mat, double *p1, double *p2, double *p3) {
  mat_add_column(mat, p1);
  mat_add_column(mat, p2);
  mat_add_column(mat, p3);
}
