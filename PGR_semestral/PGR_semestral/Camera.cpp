#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
	this->m_position = position;
	this->m_front = direction;
	this->m_up = up;
	this->m_speed = 3.5f;
	this->yaw = -90.0f;
	this->pitch = 0.0f;
	this->sensitivity = 0.08f;
}

glm::mat4 Camera::getViewMatrix() {
	glm::mat4* view = new glm::mat4();
	glm::vec3 direction;

	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	m_front = glm::normalize(direction);

	return glm::lookAt(m_position, m_position + m_front, m_up);
}
    
void Camera::forward(float deltaTime) {
	this->m_position += m_front * m_speed * deltaTime;
}
void Camera::back(float deltaTime) {
	this->m_position -= m_front * m_speed * deltaTime;
}
void Camera::left(float deltaTime) {
	this->m_position -= glm::normalize(glm::cross(m_front, m_up)) * m_speed * deltaTime;
}
void Camera::right(float deltaTime) {
	this->m_position += glm::normalize(glm::cross(m_front, m_up)) * m_speed * deltaTime;
}
void Camera::updateFrontVec(float xoffset, float yoffset) {
	yaw += sensitivity * xoffset;
	pitch += sensitivity * yoffset;
	// make sure the camera is not flipping, lock the top and bottom view angle so that
	// we can look up to the sky or to the ground but not further
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;
}
