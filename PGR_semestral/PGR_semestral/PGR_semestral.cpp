/**
 * \file hello-world.cpp
 * \brief Your first OpenGL application.
 * \author Tomas Barak
 */

#include <iostream>
#include "imgui.h"
#include "Camera.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"
#include "Mesh.h"
#include "Shader.h"
#include "Rock_7.h"
#include "pgr.h"
#include "Ancient_portal_one_mesh.h"

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
Mesh* stoneGeometry = NULL;
Mesh* stone2Geometry = NULL;

MeshGeometry* manualRockGeometry = NULL;
MeshGeometry* manualPortalGeometry = NULL;

SCommonShaderProgram shaderProgram;

const char* ROCK_MODEL_NAME = "data/rock_monolyth/mesh/magic_idol_mesh.FBX";
const char* FLOOR_MODEL_NAME = "data/floor/floor_lod_2.FBX";
const char* PORTAL_MODEL_NAME = "data/ancient_portal/Ancient_portal_one_mesh.FBX";
const char* ROCK_WALL_MODEL_NAME = "data/rock_wall/rock_wall.FBX";
const char* PALM_MODEL_NAME = "data/palm/palm.obj";
const char* STONE_MODEL_NAME = "data/rocks_1/rocks_1/Rock_7/Rock_7/Rock_7.FBX";
const char* STONE2_MODEL_NAME = "data/rock_2/Cliff_Rock_One_FBX.FBX";

const int WIN_WIDTH = 1920;
const int WIN_HEIGHT = 1080;
const char* WIN_TITLE = "Hello World";


static float xrotation = 224.6;
static float yrotation = 0;
static float zrotation = 87.8;
static float pressDelay = -1;
static bool mouseControl = false;
static bool firstframe = true;
static glm::vec2 mousePos;
static bool show_demo_window = false;
static bool show_another_window = true;
static ImVec4 clear_color = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

static float light_angle = 0.0f;

void timerCallback(int) {
    glutTimerFunc(16.6, timerCallback, 0);
    glutPostRedisplay();
}

void processInput() {
	ImGuiIO& io = ImGui::GetIO();
	pressDelay -= io.DeltaTime;

	if (io.KeysDown['q'] && pressDelay < 0)
	{
		pressDelay = 0.5f;
		mouseControl = !mouseControl;
		if(mouseControl == true) glutSetCursor(GLUT_CURSOR_NONE);
		else glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
	}
	if (firstframe) {
		mousePos = glm::vec2(io.MousePos.x, io.MousePos.y);
		firstframe = false;
	}

	if ((io.MousePos.x != mousePos.x || io.MousePos.y != mousePos.y) && mouseControl)
	{
		float xoffset = io.MousePos.x - mousePos.x;
		float yoffset = mousePos.y - io.MousePos.y;

		camera->updateFrontVec(xoffset, yoffset);

		glutWarpPointer(io.DisplaySize.x / 2, io.DisplaySize.y / 2);
		mousePos = glm::vec2(io.DisplaySize.x/2, io.DisplaySize.y/2);
	}

	if (io.KeysDown['2']) {
		camera->switchToStatic(2);
	}
	if (io.KeysDown['1']) {
		camera->switchToStatic(1);
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
	if (io.KeysDown[' '])
	{
		camera->up(io.DeltaTime);
	}
	if (io.KeyCtrl)
	{
		camera->down(io.DeltaTime);
	}
}

void my_display_code()
{
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("use WASD to move around");
		ImGui::Text("use SPACE to fly up CTRL to fly downwards");
		ImGui::Text("press \"Q\" to enable cursor and disable mouse control");  // Display some text (you can use a format strings too)
		//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Camera Parameters", &show_another_window);

		ImGui::SliderFloat("light angle", &light_angle, 0.0f, 360.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Switch to camera 1"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			camera->switchToStatic(1);
		ImGui::SameLine();
		if (ImGui::Button("Switch to camera 2"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			camera->switchToStatic(2);

		ImGui::SliderFloat("portalx angle", &xrotation, 0.0f, 360.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::SliderFloat("portaly angle", &yrotation, 0.0f, 360.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::SliderFloat("portalz angle", &zrotation, 0.0f, 360.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		glm::vec3 cameraPos = camera->getPos();
		ImGui::Begin("Camera Parameters", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		if (ImGui::Button("Close"))
			show_another_window = false;
		ImGui::Text("Camera position is");
		ImGui::Text("x: %f", cameraPos.x);
		ImGui::SameLine();
		ImGui::Text("y: %f", cameraPos.y);
		ImGui::SameLine();
		ImGui::Text("z: %f", cameraPos.z);
		ImGui::Text("Yaw: %f   Pitch: %f", camera->getYaw(), camera->getPitch());
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

	stoneGeometry = new Mesh(STONE_MODEL_NAME);
	stoneGeometry->linkShader(shaderProgram);

	stone2Geometry = new Mesh(STONE2_MODEL_NAME);
	stone2Geometry->linkShader(shaderProgram);
	//palmGeometry = new Mesh(PALM_MODEL_NAME);
	//palmGeometry->linkShader(shaderProgram);
	/*if (loadSingleMesh(ROCK_MODEL_NAME, shaderProgram, &rockGeometry) != true) {
		std::cerr << "Rock model loading failed";
	}
	CHECK_GL_ERROR();*/
	// -------------------------------- MANUAL LOAD --------------------------------------------
	manualRockGeometry = new MeshGeometry;

	CHECK_GL_ERROR();
	glGenVertexArrays(1, &manualRockGeometry->vertexArrayObject);
	glBindVertexArray(manualRockGeometry->vertexArrayObject);

	glGenBuffers(1, &manualRockGeometry->vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, manualRockGeometry->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * rock_7NVertices, rock_7Vertices, GL_STATIC_DRAW);
	
	glGenBuffers(1, &manualRockGeometry->elementBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, manualRockGeometry->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned) * rock_7NTriangles, rock_7Triangles, GL_STATIC_DRAW);
	manualRockGeometry->numTriangles = rock_7NTriangles;
	
	CHECK_GL_ERROR();
	//glUseProgram(shaderProgram.program);

	glEnableVertexAttribArray(shaderProgram.posLocation);
	glVertexAttribPointer(shaderProgram.posLocation, 3, GL_FLOAT, GL_FALSE, 8* sizeof(float), 0);

	glEnableVertexAttribArray(shaderProgram.normalLocation);
	glVertexAttribPointer(shaderProgram.normalLocation, 3, GL_FLOAT, GL_FALSE, 8* sizeof(float), (void*)(3*sizeof(float)));

	CHECK_GL_ERROR();
	glBindVertexArray(0);

	manualPortalGeometry= new MeshGeometry;

	CHECK_GL_ERROR();
	glGenVertexArrays(1, &manualPortalGeometry->vertexArrayObject);
	glBindVertexArray(manualPortalGeometry->vertexArrayObject);

	glGenBuffers(1, &manualPortalGeometry->vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, manualPortalGeometry->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * cube_001NVertices, cube_001Vertices, GL_STATIC_DRAW);
	
	glGenBuffers(1, &manualPortalGeometry->elementBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, manualPortalGeometry->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned) * cube_001NTriangles,cube_001Triangles , GL_STATIC_DRAW);
	manualPortalGeometry->numTriangles = cube_001NTriangles;
	
	CHECK_GL_ERROR();
	//glUseProgram(shaderProgram.program);

	glEnableVertexAttribArray(shaderProgram.posLocation);
	glVertexAttribPointer(shaderProgram.posLocation, 3, GL_FLOAT, GL_FALSE, 8* sizeof(float), 0);

	glEnableVertexAttribArray(shaderProgram.normalLocation);
	glVertexAttribPointer(shaderProgram.normalLocation, 3, GL_FLOAT, GL_FALSE, 8* sizeof(float), (void*)(3*sizeof(float)));

	CHECK_GL_ERROR();
	glBindVertexArray(0);
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
	glm::mat4 modelMatrix;
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), io.DisplaySize.x / io.DisplaySize.y,0.1f, 100.0f);
	modelMatrix = glm::mat4(1.0f);

	glm::mat4 NormalMatrix = glm::transpose(glm::inverse(modelMatrix));
	glm::vec3 lightPos = glm::vec3(20.0f*glm::sin(glm::radians(light_angle)), 20.0f, 20.0f*glm::cos(glm::radians(light_angle)));
	
	double time = glutGet(GLUT_ELAPSED_TIME);

	glUniformMatrix4fv(shaderProgram.ModelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(shaderProgram.NormalModelLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
	glUniform3fv(shaderProgram.lightLocation, 1, glm::value_ptr(lightPos));

	for (int i = 0; i < 10; i++) {
		int x_coord = -2;
		if (i < 5) x_coord = 2;

		modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x_coord, 0, -(i % 5)* 2));
		NormalMatrix = glm::transpose(glm::inverse(modelMatrix));

		glUniformMatrix4fv(shaderProgram.ModelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(shaderProgram.NormalModelLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
		rockGeometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);
	}

		
	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 2.7f, -15.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(8.0f, 8.0f, 8.0f));
	NormalMatrix = glm::transpose(glm::inverse(modelMatrix));

	glUniformMatrix4fv(shaderProgram.ModelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(shaderProgram.NormalModelLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
	rockWallGeometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-5.0f, -1.0f, 2.0f));
	NormalMatrix = glm::transpose(glm::inverse(modelMatrix));
	CHECK_GL_ERROR();

	glUniformMatrix4fv(shaderProgram.ModelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(shaderProgram.NormalModelLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
	stoneGeometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(5.0f, -1.0f, 2.0f));
	NormalMatrix = glm::transpose(glm::inverse(modelMatrix));

	glUniformMatrix4fv(shaderProgram.ModelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(shaderProgram.NormalModelLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
	stone2Geometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);
	//palmGeometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);
	/*
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.0f, 20.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(40.0f, 40.0f, 40.0f));
	groundGeometry->drawMesh(shaderProgram, camera, modelMatrix, projectionMatrix);
	*/
	//---------------------MANUAL LOAD DRAW-------------------------------------
	CHECK_GL_ERROR();
	glUseProgram(shaderProgram.program);
	CHECK_GL_ERROR();
	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-5.0f, -1.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.001, 0.001, 0.001));
	NormalMatrix = glm::transpose(glm::inverse(modelMatrix));
	glm::mat4 PVM = projectionMatrix * camera->getViewMatrix() * modelMatrix;


	glUniformMatrix4fv(shaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVM));
	glUniformMatrix4fv(shaderProgram.ModelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(shaderProgram.NormalModelLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

	glBindVertexArray(manualRockGeometry->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, manualRockGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

	CHECK_GL_ERROR();
	glUseProgram(shaderProgram.program);
	CHECK_GL_ERROR();
	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-5.0f, -1.0f, 20.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(xrotation), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(yrotation), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(zrotation), glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(10, 10, 10));
	NormalMatrix = glm::transpose(glm::inverse(modelMatrix));
	PVM = projectionMatrix * camera->getViewMatrix() * modelMatrix;


	glUniformMatrix4fv(shaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVM));
	glUniformMatrix4fv(shaderProgram.ModelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(shaderProgram.NormalModelLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

	glBindVertexArray(manualPortalGeometry->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, manualPortalGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

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
