#pragma once
#include "pgr.h"

typedef struct _commonShaderProgram {
	GLuint program;
	GLint posLocation;
	GLint normalLocation;

	GLint PVMmatrixLocation;
	GLint ModelLocation;
	GLint NormalModelLocation;
	GLint lightLocation;
} SCommonShaderProgram;
