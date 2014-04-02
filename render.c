#include "display.h"
#include "transform.h"
#include "render.h"

// assumes x y z 1 pattern
char should_draw(double coors[12], double *eye) {
  //backface culling
  double *p1 = coors, *p2 = coors+4, *p3 = coors+8;
  double a[3], b[3], cross[3];
  a[0] = p2[0]-p1[0]; //p2-p1
  a[1] = p2[1]-p1[1];
  a[2] = p2[2]-p1[2];
  b[0] = p3[0]-p2[0]; //p3-p2
  b[1] = p3[1]-p2[1];
  b[2] = p3[2]-p2[2];
  cross[0] = (a[1]*b[2]) - (a[2]*b[1]);
  cross[1] = (a[2]*b[0]) - (a[0]*b[2]);
  cross[2] = (a[0]*b[1]) - (a[1]*b[0]);
  double ep1[3];
  ep1[0] = p1[0] - eye[0]; //p1 - eye
  ep1[1] = p1[1] - eye[1];
  ep1[2] = p1[2] - eye[2];
  double dot = (cross[0]*ep1[0]) + (cross[1]*ep1[1]) + (cross[2]*ep1[2]);
  return dot < 0;
}

void renderperspective(Matrix *faces, double *eye, uint32_t color) {
  int mapped[6];
  double coors[6];
  int c;
  double ex = eye[0], ey = eye[1], ez = eye[2];
  //printf("rendering perspective: %.2f %.2f %.2f\n", ex, ey, ez);
  double pz;
  int line[4];
  double tri[12];
  for (c = 0; c < faces->cols; c += 3) {
    mat_get_column(faces, c, tri);
    mat_get_column(faces, c+4, tri);
    mat_get_column(faces, c+8, tri);
    /* if (!should_draw(tri, eye)) */
    /*   break; */
    pz = mat_get_cell(faces, c, 2);
    if (pz > ez)
      break;
    coors[0] = ex - (ez * (mat_get_cell(faces, c, 0)-ex) / (pz - ez));
    coors[1] = ey - (ez * (mat_get_cell(faces, c, 1)-ey) / (pz - ez));
    pz = mat_get_cell(faces, c+1, 2);
    if (pz > ez)
      break;
    coors[2] = ex - (ez * (mat_get_cell(faces, c+1, 0)-ex) / (pz - ez));
    coors[3] = ey - (ez * (mat_get_cell(faces, c+1, 1)-ey) / (pz - ez));
    pz = mat_get_cell(faces, c+2, 2);
    if (pz > ez)
      break;
    coors[4] = ex - (ez * (mat_get_cell(faces, c+2, 0)-ex) / (pz - ez));
    coors[5] = ey - (ez * (mat_get_cell(faces, c+2, 1)-ey) / (pz - ez));

    map_coors(coors, coors+1);
    map_coors(coors+2, coors+3);
    map_coors(coors+4, coors+5);

    // line 1
    line[0] = coors[0];
    line[1] = coors[1];
    line[2] = coors[2];
    line[3] = coors[3];
    dline(line, color);

    // line 2
    line[0] = coors[4];
    line[1] = coors[5];
    dline(line, color);

    // line 3
    line[2] = coors[0];
    line[3] = coors[1];
    dline(line, color);
  }
}

void rendercyclops(Matrix *faces, double *eye) {
  clear_screen();
  renderperspective(faces, eye, rgb(255, 255, 255));
  update_display();
}

void renderstereo(Matrix *faces, double *eyes) {
  clear_screen();

  // left -- red
  renderperspective(faces, eyes, rgb(127, 0, 0));

  // right -- cyan
  mixcolors(1);
  renderperspective(faces, eyes+3, rgb(0, 127, 127));
  mixcolors(0);

  update_display();
}

char endspin() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if ( event.type == SDL_QUIT ) {
      printf("Memory leaking... You really ought to fix that.\n");
      /* SDL_DestroyRenderer(ren); */
      /* SDL_DestroyWindow(win); */
      SDL_Quit();
      return 1;
    } else if (event.type == SDL_KEYDOWN) {
      return 1;
    }
  }
  return 0;
}

Matrix *spinmat(int x, int y, int z) {
  Matrix *xr = rotate_x_mat(x * M_PI / 180);
  Matrix *yr = rotate_y_mat(y * M_PI / 180);
  Matrix *zr = rotate_z_mat(z * M_PI / 180);
  Matrix *temp = mat_multiply(xr, yr);
  Matrix *xyz = mat_multiply(temp, zr);
  mat_destruct(temp);
  mat_destruct(xr);
  mat_destruct(yr);
  mat_destruct(zr);
  return xyz;
}

void spincyclops(Matrix *faces, double *eye) {
  Matrix *xyz = spinmat(1, 1, 1);
  Matrix *rot;
  uint32_t color = rgb(0, 200, 0);
  uint32_t black = rgb(0, 0, 0);
  clear_screen();
  while(!endspin()) {
    renderperspective(faces, eye, black);
    rot = mat_multiply(xyz, faces);
    mat_destruct(faces);
    faces = rot;
    renderperspective(faces, eye, color);
    SDL_Delay(50);
    update_display();
  }
}

void spinstereo(Matrix *faces, double *eyes) {
  Matrix *xyz = spinmat(1, 1, 1);
  double *el = eyes;
  double *er = eyes + 3;
  Matrix *rot;
  uint32_t red = rgb(127, 0, 0);
  uint32_t cyan = rgb(0, 127, 127);
  uint32_t black = rgb(0, 0, 0);
  clear_screen();
  while(!endspin()) {
    renderperspective(faces, el, black);
    renderperspective(faces, er, black);
    rot = mat_multiply(xyz, faces);
    mat_destruct(faces);
    faces = rot;
    renderperspective(faces, el, red);
    mixcolors(1);
    renderperspective(faces, er, cyan);
    mixcolors(0);
    SDL_Delay(50);
    update_display();
  }
}
