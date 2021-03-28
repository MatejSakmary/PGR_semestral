#pragma once
#include "pgr.h"
#include "Shader.h"
#include "Camera.h"
#include <iostream>

class Mesh
{
public:
	GLuint vertexBufferObject;
	GLuint elementBufferObject;
	GLuint vertexArrayObject;
	unsigned int numTriangles;


	Mesh(const std::string& filename);
	void linkShader(SCommonShaderProgram& shader);
	void drawMesh(SCommonShaderProgram& shader, Camera* camera, glm::mat4 modelMatrix, glm::mat4 projectionMatrix);

private:
	bool loadSingleMesh(const std::string&);
	unsigned long numVerticies;
};

