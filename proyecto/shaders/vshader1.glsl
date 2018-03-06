#version 130

attribute vec3 vPosition;

uniform mat4 model_view;
uniform mat4 projection;

void main() {
  vec4 vPosition4 = vec4(vPosition, 1.0);
  gl_Position = projection * model_view * vPosition4;
} 
