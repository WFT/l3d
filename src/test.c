#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "matrix.h"
#include "transform.h"
#include "objects.h"
#include "display.h"

void rand_point(double p[4]) {
	int i;
	for (i=0; i < 3; i++)
		p[i] = rand() % 200 - 100;
	p[3] = 1;
}

double rand_radian() {
	return ((double)rand()/(double)RAND_MAX) * 6.29;
}

void spin_test() {
	Matrix *faces = mat_construct(0, 4);
	// ~180 degrees
	double col[4] = {0, 0, 0, 1};
	clock_t t;
	Matrix *spin;
	double rad;
	unsigned long i;
	do {
		for (i=0; i < 3000; i++) {
			rand_point(col);
			mat_add_column(faces, col);
		}
		rad = rand_radian();
		spin = rotate_x_mat(rad);
		t = clock();
		apply_transform(spin, &faces);
		t = clock() - t;
		printf("%d faces spun (%f radians) in %f seconds\n", faces->cols/3, rad, ((float)t)/CLOCKS_PER_SEC);
	} while (((float)t)/CLOCKS_PER_SEC < 1);
	mat_destruct(faces);
}

int main(int argc, char **argv) {
	srand(time(NULL));
	spin_test();
	return 0;
}