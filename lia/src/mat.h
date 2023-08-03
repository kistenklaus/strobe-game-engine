#ifndef STROBE_LIA_MAT_H
#define STROBE_LIA_MAT_H

typedef struct {
  float m00;
  float m10;
  float m20;
  float m30;
  float m01;
  float m11;
  float m21;
  float m31;
  float m02;
  float m12;
  float m22;
  float m32;
  float m03;
  float m13;
  float m23;
  float m33;
} mat4f;

typedef struct {
  float m00;
  float m10;
  float m20;
  float m01;
  float m11;
  float m21;
  float m02;
  float m12;
  float m22;
} mat3f;

typedef struct {
  float m00;
  float m10;
  float m01;
  float m11;
} mat2f;

void mat4f_identity(mat4f *mat4);

void mat3f_identity(mat4f *mat3);

void mat2f_identity(mat2f *mat2);

void mat4f_mul(mat4f *r, mat4f *a, mat4f *b);

#endif
