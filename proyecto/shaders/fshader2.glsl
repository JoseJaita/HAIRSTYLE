#version 120

varying vec3 vert_color;
//out vec4 fColor;

void main() { 
  gl_FragColor = vec4(vert_color,1.0);
} 

