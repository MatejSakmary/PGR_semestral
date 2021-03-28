#pragma once

#include "pgr.h"
#include <iostream>

class Camera
{
private:
	glm::vec3 m_position;
	glm::vec3 m_front;
	glm::vec3 m_up;
	float pitch;
	float yaw;
	float m_speed;
	float sensitivity;

public:
	Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 m_up);
	void forward(float deltaTime);
	void back(float deltaTime);
	void left(float deltaTime);
	void right(float deltaTime);
	void updateFrontVec(float xoffset, float yoffset);
	glm::mat4 getViewMatrix();
};

