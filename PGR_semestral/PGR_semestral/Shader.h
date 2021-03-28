#pragma once
#include "pgr.h"

typedef struct _commonShaderProgram {
	GLuint program;
	GLint posLocation;
	GLint colorLocation;
	GLint PVMmatrixLocation;
} SCommonShaderProgram;
