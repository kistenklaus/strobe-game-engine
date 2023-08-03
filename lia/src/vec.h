#ifndef STROBE_LIA_VEC2_H
#define STROBE_LIA_VEC2_H

typedef struct {
  float x;
  float y;
} vec2f;

typedef struct {
  float x;
  float y;
  float z;
} vec3f;

typedef struct {
  float x;
  float y;
  float z;
  float w;
} vec4f;

typedef struct {
  int x;
  int y;
} vec2i;

typedef struct {
  int x;
  int y;
  int z;
} vec3i;

typedef struct {
  int x;
  int y;
  int z;
  int w;
} vec4i;

void vec2f_add(vec2f *s , vec2f *a, vec2f *b);
void vec3f_add(vec3f *s, vec3f *a, vec3f *b);
void vec4f_add(vec4f *s, vec4f *a, vec4f *b);
float vec2f_dot(vec2f *a, vec2f *b);
float vec3f_dot(vec3f *a, vec3f *b);
float vec4f_dot(vec4f *a, vec4f *b);
void vec3i_add(vec3i *s, vec3i *a, vec3i *b);
void vec2i_add(vec2i *s, vec2i *a, vec2i *b);
void vec4i_add(vec4i *s, vec4i *a, vec4i *b);
int vec2i_dot(vec2i *a, vec2i *b);
int vec3i_dot(vec3i *a, vec3i *b);
int vec4i_dot(vec4i *a, vec4i *b);

#endif
