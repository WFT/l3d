#pragma once
typedef struct matrix {
  int cols, rows;
  double **cells;
} Matrix;

void pmat(Matrix *);
double mat_get_cell(struct matrix *mat, int c, int r);
void mat_set_cell(struct matrix *mat, int c, int r, double val);
// c : number of columns to add
struct matrix * mat_construct(int c, int r);

// mat : pointer to matrix to free
void mat_destruct(struct matrix *mat);

// mat : pointer to matrix to add to, col : array of doubles to add
void mat_add_column(struct matrix *mat, double *col);

// multiply a by b
struct matrix * mat_multiply(struct matrix *a, struct matrix *b);

void mat_multinmat(struct matrix *a, struct matrix *b, struct matrix *res);

void mat_extend(struct matrix *dest, struct matrix *src);

void mat_resize(struct matrix *mat, int c, int r);
