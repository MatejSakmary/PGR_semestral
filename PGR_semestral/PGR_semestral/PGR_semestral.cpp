/**
 * \file hello-world.cpp
 * \brief Your first OpenGL application.
 * \author Tomas Barak
 */

#include "imgui.h"
#include "Camera.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"
#include "Mesh.h"
#include "Shader.h"
#include <iostream>
#include "pgr.h"

#define SCENE_WIDTH 1.0f
#define SCENE_HEIGHT 1.0f
#define SCENE_DEPTH 1.0f


typedef struct MeshGeometry {
	GLuint vertexBufferObject;
	GLuint elementBufferObject;
	GLuint vertexArrayObject;
	unsigned int numTriangles;
} MeshGeometry;

Camera* camera = NULL;
MeshGeometry* rockGeometry;
SCommonShaderProgram shaderProgram;
const char* ROCK_MODEL_NAME = "data/rock_monolyth/mesh/magic_idol_mesh.FBX";

const int WIN_WIDTH = 1080;
const int WIN_HEIGHT = 1080;
const char* WIN_TITLE = "Hello World";


static bool firstframe = true;;
static glm::vec2 mousePos;
static bool show_demo_window = false;
static bool show_another_window = true;
static ImVec4 clear_color = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

void timerCallback(int) {
    glutTimerFunc(16.6, timerCallback, 0);
    glutPostRedisplay();
}

void reshapeCallback(int newWidth, int newHeight) {
    glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

void processInput() {
	ImGuiIO& io = ImGui::GetIO();

	if (firstframe) {
		mousePos = glm::vec2(io.MousePos.x, io.MousePos.y);
		firstframe = false;
	}

	if (io.MousePos.x != mousePos.x || io.MousePos.y != mousePos.y)
	{
		float xoffset = io.MousePos.x - mousePos.x;
		float yoffset = mousePos.y - io.MousePos.y;

		camera->updateFrontVec(xoffset, yoffset);

		glutWarpPointer(io.DisplaySize.x / 2, io.DisplaySize.y / 2);
		mousePos = glm::vec2(io.DisplaySize.x/2, io.DisplaySize.y/2);
	}
	if (io.KeysDown['w']) {
		camera->forward(io.DeltaTime);
	}
	if (io.KeysDown['s']) {
		camera->back(io.DeltaTime);
	}
	if (io.KeysDown['a']) {
		camera->left(io.DeltaTime);
	}
	if (io.KeysDown['d']) {
		camera->right(io.DeltaTime);
	}
}

bool loadSingleMesh(const std::string& fileName, SCommonShaderProgram& shader, MeshGeometry** geometry) {
	Assimp::Importer importer;

	// Unitize object in size (scale the model to fit into (-1..1)^3)
	importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, 1);

	// Load asset from the file - you can play with various processing steps
	const aiScene* scn = importer.ReadFile(fileName.c_str(), 0
		| aiProcess_Triangulate             // Triangulate polygons (if any).
		| aiProcess_PreTransformVertices    // Transforms scene hierarchy into one root with geometry-leafs only. For more see Doc.
		| aiProcess_GenSmoothNormals        // Calculate normals per vertex.
		| aiProcess_JoinIdenticalVertices);

	// abort if the loader fails
	if (scn == NULL) {
		std::cerr << "assimp error: " << importer.GetErrorString() << std::endl;
		*geometry = NULL;
		return false;
	}

	// some formats store whole scene (multiple meshes and materials, lights, cameras, ...) in one file, we cannot handle that in our simplified example
	if (scn->mNumMeshes != 1) {
		std::cerr << "this simplified loader can only process files with only one mesh" << std::endl;
		*geometry = NULL;
		return false;
	}

	// in this phase we know we have one mesh in our loaded scene, we can directly copy its data to OpenGL ...
	const aiMesh* mesh = scn->mMeshes[0];

	*geometry = new MeshGeometry;

	// vertex buffer object, store all vertex positions and normals
	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * mesh->mNumVertices, 0, GL_STATIC_DRAW); // allocate memory for vertices, normals, and texture coordinates
	// first store all vertices
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * mesh->mNumVertices, mesh->mVertices);
	// then store all normals
	glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float) * mesh->mNumVertices, 3 * sizeof(float) * mesh->mNumVertices, mesh->mNormals);

	// just texture 0 for now
	float* textureCoords = new float[2 * mesh->mNumVertices];  // 2 floats per vertex
	float* currentTextureCoord = textureCoords;

	// copy texture coordinates
	aiVector3D vect;

	if (mesh->HasTextureCoords(0)) {
		// we use 2D textures with 2 coordinates and ignore the third coordinate
		for (unsigned int idx = 0; idx < mesh->mNumVertices; idx++) {
			vect = (mesh->mTextureCoords[0])[idx];
			*currentTextureCoord++ = vect.x;
			*currentTextureCoord++ = vect.y;
		}
	}

	// finally store all texture coordinates
	glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(float) * mesh->mNumVertices, 2 * sizeof(float) * mesh->mNumVertices, textureCoords);

	// copy all mesh faces into one big array (assimp supports faces with ordinary number of vertices, we use only 3 -> triangles)
	unsigned int* indices = new unsigned int[mesh->mNumFaces * 3];
	for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
		indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
		indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
		indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
	}

	// copy our temporary index array to OpenGL and free the array
	glGenBuffers(1, &((*geometry)->elementBufferObject));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned) * mesh->mNumFaces, indices, GL_STATIC_DRAW);

	delete[] indices;

	// copy the material info to MeshGeometry structure
	const aiMaterial* mat = scn->mMaterials[mesh->mMaterialIndex];
	aiColor4D color;
	aiString name;
	aiReturn retValue = AI_SUCCESS;

	// Get returns: aiReturn_SUCCESS 0 | aiReturn_FAILURE -1 | aiReturn_OUTOFMEMORY -3
	mat->Get(AI_MATKEY_NAME, name); // may be "" after the input mesh processing. Must be aiString type!

	if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color)) != AI_SUCCESS)
		color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);


	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject); // bind our element array buffer (indices) to vao
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);

	CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader.posLocation);
	glVertexAttribPointer(shader.posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//glDisableVertexAttribArray(shader.colorLocation);
	CHECK_GL_ERROR();
	// following line is problematic on AMD/ATI graphic cards
	// -> if you see black screen (no objects at all) than try to set color manually in vertex shader to see at least something
	//glVertexAttrib3f(shader.colorLocation, color.r, color.g, color.b);
	CHECK_GL_ERROR();

	glBindVertexArray(0);

	(*geometry)->numTriangles = mesh->mNumFaces;

	return true;
}

void my_display_code()
{
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}
}

void init() {
	camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f),
				        glm::vec3(0.0f, 0.0f, -1.0f),
				        glm::vec3(0.0f, 1.0f, 0.0f));
    glClearColor(0.2f, 0.1f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

    GLuint shaders[] = {
      pgr::createShaderFromFile(GL_VERTEX_SHADER, "vertex.shader"),
      pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "fragment.shader"),
      0
    };

    shaderProgram.program = pgr::createProgram(shaders);
	shaderProgram.posLocation = glGetAttribLocation(shaderProgram.program, "position");
	shaderProgram.PVMmatrixLocation = glGetUniformLocation(shaderProgram.program, "PVMmatrix");

	if (loadSingleMesh(ROCK_MODEL_NAME, shaderProgram, &rockGeometry) != true) {
		std::cerr << "Rock model loading failed";
	}
	CHECK_GL_ERROR();
}

void draw() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    my_display_code();

    ImGuiIO& io = ImGui::GetIO();  
	processInput();
	glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Render();


    glUseProgram(shaderProgram.program);
	glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), 1080.0f / 1080.f,0.1f, 100.0f);
	
	glm::mat4* viewMatrix = camera->getViewMatrix();

	glm::mat4 projectionMatrix = ProjectionMatrix;
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

	glm::mat4 PVM = projectionMatrix * (*viewMatrix) * modelMatrix;
	glUniformMatrix4fv(shaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVM));

    glBindVertexArray(rockGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, rockGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitContextVersion(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutCreateWindow(WIN_TITLE);

    glutDisplayFunc(draw);

    glutTimerFunc(16.6, timerCallback, 0);
    if (!pgr::initialize(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR))
        pgr::dieWithError("pgr init failed, required OpenGL not supported?");

    init();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

	glutSetCursor(GLUT_CURSOR_NONE);
    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL3_Init();

    std::cout << "Hello triangle!" << std::endl;

    glutMainLoop();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
