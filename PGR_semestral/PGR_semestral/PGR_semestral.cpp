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
Mesh* rockGeometry = NULL;
Mesh* groundGeometry = NULL;
Mesh* portalGeometry = NULL;
Mesh* rockWallGeometry = NULL;
Mesh* palmGeometry = NULL;
SCommonShaderProgram shaderProgram;

const char* ROCK_MODEL_NAME = "data/rock_monolyth/mesh/magic_idol_mesh.FBX";
const char* FLOOR_MODEL_NAME = "data/floor/floor_lod_2.FBX";
const char* PORTAL_MODEL_NAME = "data/ancient_portal/Ancient_portal.FBX";
const char* ROCK_WALL_MODEL_NAME = "data/rock_wall/rock_wall.FBX";
const char* PALM_MODEL_NAME = "data/palm/palm.obj";

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

	CHECK_GL_ERROR();
    shaderProgram.program = pgr::createProgram(shaders);
	shaderProgram.posLocation = glGetAttribLocation(shaderProgram.program, "position");
	shaderProgram.normalLocation = glGetAttribLocation(shaderProgram.program, "aNormal");

	shaderProgram.PVMmatrixLocation = glGetUniformLocation(shaderProgram.program, "PVMmatrix");
	shaderProgram.ModelLocation = glGetUniformLocation(shaderProgram.program, "Model");
	shaderProgram.NormalModelLocation = glGetUniformLocation(shaderProgram.program, "NormalModel");
	shaderProgram.lightLocation = glGetUniformLocation(shaderProgram.program, "lightPos");
	CHECK_GL_ERROR();

	rockGeometry = new Mesh(ROCK_MODEL_NAME);
	rockGeometry->linkShader(shaderProgram);

	groundGeometry = new Mesh(FLOOR_MODEL_NAME);
	rockGeometry->linkShader(shaderProgram);

	//portalGeometry = new Mesh(PORTAL_MODEL_NAME);
	//portalGeometry->linkShader(shaderProgram);

	rockWallGeometry = new Mesh(ROCK_WALL_MODEL_NAME);
	rockWallGeometry->linkShader(shaderProgram);

	palmGeometry = new Mesh(PALM_MODEL_NAME);
	palmGeometry->linkShader(shaderProgram);
	/*if (loadSingleMesh(ROCK_MODEL_NAME, shaderProgram, &rockGeometry) != true) {
		std::cerr << "Rock model loading failed";
	}
	CHECK_GL_ERROR();*/
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


	glm::mat4 modelMatrix;
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 1080.0f / 1080.f,0.1f, 100.0f);
	modelMatrix = glm::mat4(1.0f);

	glUseProgram(shaderProgram.program);
	glm::mat4 NormalMatrix = glm::transpose(glm::inverse(modelMatrix));
	
	double time = glutGet(GLUT_ELAPSED_TIME);
	glm::vec3 lightPos = glm::vec3(10.0f*sin(time/5000), 10.0f, 10.0f*cos(time/5000));

	glUniformMatrix4fv(shaderProgram.ModelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(shaderProgram.NormalModelLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
	glUniform3fv(shaderProgram.lightLocation, 1, glm::value_ptr(lightPos));

	//rockGeometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);

	
	for (int i = 0; i < 10; i++) {
		int x_coord = -2;
		if (i < 5) x_coord = 2;

		modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x_coord, 0, -(i % 5)* 2));
		rockGeometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);
	}

		
	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 2.7f, -15.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(8.0f, 8.0f, 8.0f));

	rockWallGeometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);

	modelMatrix = glm::mat4(1.0f);
	palmGeometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);
	/*
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.0f, 20.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(40.0f, 40.0f, 40.0f));
	groundGeometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);
	*/
	

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
