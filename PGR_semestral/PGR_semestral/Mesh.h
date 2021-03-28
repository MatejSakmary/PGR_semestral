#pragma once
#include "pgr.h"
#include "Shader.h"
#include <iostream>

class Mesh
{
public:
	GLuint vertexBufferObject;
	GLuint elementBufferObject;
	GLuint vertexArrayObject;
	unsigned int numTriangles;


	Mesh(const std::string& filename);
	bool linkShader();
private:
	bool loadSingleMesh(const std::string&);
};

