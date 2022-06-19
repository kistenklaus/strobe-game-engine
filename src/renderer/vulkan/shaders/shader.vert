#version 450

layout(location = 0) in vec2 v_position;
layout(location=1) in vec3 v_color;

layout(location = 0) out vec3 in_color;


void main(void) {
  in_color = v_color;
  gl_Position = vec4(v_position.xy, 0.0, 1.0);
}
