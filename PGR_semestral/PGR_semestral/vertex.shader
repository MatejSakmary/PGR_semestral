#version 330 core

in vec3 position;
in vec3 aNormal;

uniform mat4 PVMmatrix;
uniform mat4 Model;
uniform mat4 NormalModel;

out vec3 fragPos;
out vec3 fragNormal;

void main() 
{
  gl_Position = PVMmatrix * vec4(position, 1.0f);
  fragPos = vec3(Model * vec4(position, 1.0f));
  fragNormal = vec3(NormalModel * vec4(aNormal,0.0f));
}
