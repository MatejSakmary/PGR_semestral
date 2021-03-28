#version 140
uniform mat4 PVMmatrix;
in vec3 position;
out vec3 myColor;

void main() 
{
  myColor = vec3(0.3, 0.3, 0.3);
  gl_Position =  PVMmatrix * vec4(position, 1.0f);
}
