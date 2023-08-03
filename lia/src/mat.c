
#include "mat.h"

void mat4f_identity(mat4f *mat4) {
  mat4->m00 = 1;
  mat4->m10 = 0;
  mat4->m20 = 0;
  mat4->m30 = 0;
  mat4->m01 = 0;
  mat4->m11 = 1;
  mat4->m21 = 0;
  mat4->m31 = 0;
  mat4->m02 = 0;
  mat4->m12 = 0;
  mat4->m22 = 1;
  mat4->m32 = 0;
  mat4->m03 = 0;
  mat4->m13 = 0;
  mat4->m23 = 0;
  mat4->m33 = 1;
}

void mat3f_identity(mat4f *mat3) {
  mat3->m00 = 1;
  mat3->m10 = 0;
  mat3->m20 = 0;
  mat3->m01 = 0;
  mat3->m11 = 1;
  mat3->m21 = 0;
  mat3->m02 = 0;
  mat3->m12 = 0;
  mat3->m22 = 1;
}

void mat2f_identity(mat2f *mat2) {
  mat2->m00 = 0;
  mat2->m10 = 0;
  mat2->m01 = 0;
  mat2->m11 = 0;
}

void mat4f_mul(mat4f *r, mat4f *a, mat4f *b) {
  r->m00 =
      a->m00 * b->m00 + a->m01 * b->m10 + a->m02 * b->m20 + a->m03 * b->m30;
  r->m10 =
      a->m10 * b->m00 + a->m11 * b->m10 + a->m12 * b->m20 + a->m13 * b->m30;
  r->m20 =
      a->m20 * b->m00 + a->m21 * b->m10 + a->m22 * b->m20 + a->m23 * b->m30;
  r->m30 =
      a->m30 * b->m00 + a->m31 * b->m10 + a->m32 * b->m20 + a->m33 * b->m30;
  r->m01 =
      a->m00 * b->m01 + a->m01 * b->m11 + a->m02 * b->m21 + a->m03 * b->m31;
  r->m11 =
      a->m10 * b->m01 + a->m11 * b->m11 + a->m12 * b->m21 + a->m13 * b->m31;
  r->m21 =
      a->m20 * b->m01 + a->m21 * b->m11 + a->m22 * b->m21 + a->m23 * b->m31;
  r->m31 =
      a->m30 * b->m01 + a->m31 * b->m11 + a->m32 * b->m21 + a->m33 * b->m31;
  r->m02 =
      a->m00 * b->m02 + a->m01 * b->m12 + a->m02 * b->m22 + a->m03 * b->m32;
  r->m12 =
      a->m10 * b->m02 + a->m11 * b->m12 + a->m12 * b->m22 + a->m13 * b->m32;
  r->m22 =
      a->m20 * b->m02 + a->m21 * b->m12 + a->m22 * b->m22 + a->m23 * b->m32;
  r->m32 =
      a->m30 * b->m02 + a->m31 * b->m12 + a->m32 * b->m22 + a->m33 * b->m32;
  r->m03 =
      a->m00 * b->m03 + a->m01 * b->m13 + a->m02 * b->m23 + a->m03 * b->m33;
  r->m13 =
      a->m10 * b->m03 + a->m11 * b->m13 + a->m12 * b->m23 + a->m13 * b->m33;
  r->m23 =
      a->m20 * b->m03 + a->m21 * b->m13 + a->m22 * b->m23 + a->m23 * b->m33;
  r->m33 =
      a->m30 * b->m03 + a->m31 * b->m13 + a->m32 * b->m23 + a->m33 * b->m33;
}
