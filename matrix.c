#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "matrix.h"

void pmat(Matrix *m) {
  int r, c;
  for (r = 0; r < m->rows; r++) {
    printf("|");
    for (c = 0; c < m->cols; c++) {
      printf("%s%.2f", c==0?"":"\t", mat_get_cell(m, c, r));
    }
    printf("|\n");
  }
}

double mat_get_cell(struct matrix *mat, int c, int r) {
  assert(r < mat->rows);
  assert(c < mat->cols);
  return mat->cells[c][r];
}

void mat_set_cell(struct matrix *mat, int c, int r, double val) {
  assert(r < mat->rows);
  assert(c < mat->cols);
  mat->cells[c][r] = val;
}

// number of columns to add
struct matrix * mat_construct(int c, int r) {
  struct matrix *ret = malloc(sizeof(struct matrix));
  ret->cols = c;
  ret->rows = r;
  ret->cells = (double **)malloc(c * sizeof(double *));
  int i;
  for (i = 0; i < c; i++) {
    ret->cells[i] = (double *)calloc(r, sizeof(double));
  }
  return ret;
}

void mat_destruct(struct matrix *mat) {
  int i;
  for (i = 0; i < mat->cols; i++)
    free(mat->cells[i]);
  free(mat);
}

void mat_add_column(struct matrix *mat, double *col) {
  int c = mat->cols;
  mat->cols++;
  mat->cells = realloc(mat->cells, mat->cols * sizeof(double *));
  mat->cells[c] = col;
}

struct matrix * mat_multiply(struct matrix *a, struct matrix *b) {  
  assert(a->cols == b->rows);
  struct matrix *ret = mat_construct(b->cols, a->rows);
  int r, c, j;
  double val = 0;
  for (c = 0; c < ret->cols; c++) {
    for (r = 0; r < ret->rows; r++) {
      for (j = 0; j < ret->rows; j++) {
	val += mat_get_cell(a, j, r) * mat_get_cell(b, c, j);
      }
      mat_set_cell(ret, c, r, val);
      val = 0;
    }
  }
  return ret;
}
