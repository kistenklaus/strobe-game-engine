#include "vec.h"

void vec2f_add(vec2f *s, vec2f *a, vec2f *b) {
  s->x = a->x + b->x;
  s->y = a->y + b->y;
}

void vec3f_add(vec3f *s, vec3f *a, vec3f *b) {
  s->x = a->x + b->x;
  s->y = a->y + b->y;
  s->z = a->z + b->z;
}

void vec4f_add(vec4f *s, vec4f *a, vec4f *b) {
  s->x = a->x + b->x;
  s->y = a->y + b->y;
  s->z = a->z + b->z;
  s->w = a->w + b->w;
}

float vec2f_dot(vec2f *a, vec2f *b) { return a->x * b->x + a->y * b->y; }

float vec3f_dot(vec3f *a, vec3f *b) {
  return a->x * b->x + a->y * b->y + a->z * b->z;
}

float vec4f_dot(vec4f *a, vec4f *b) {
  return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

void vec2i_add(vec2i *s, vec2i *a, vec2i *b) {
  s->x = a->x + b->x;
  s->y = a->y + b->y;
}
void vec3i_add(vec3i *s, vec3i *a, vec3i *b) {
  s->x = a->x + b->x;
  s->y = a->y + b->y;
  s->z = a->z + b->z;
}
void vec4i_add(vec4i *s, vec4i *a, vec4i *b) {
  s->x = a->x + b->x;
  s->y = a->y + b->y;
  s->z = a->z + b->z;
  s->w = a->w + b->w;
}
int vec2i_dot(vec2i *a, vec2i *b) { return a->x * b->x + a->y * b->y; }
int vec3i_dot(vec3i *a, vec3i *b) {
  return a->x * b->x + a->y * b->y + a->z * b->z;
}
int vec4i_dot(vec4i *a, vec4i *b) {
  return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}
