#pragma once


#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Default camera values
namespace CAM_DEFAULTS {

	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float SPEED = 30.0f;
	const float SENSITIVITY = 0.1f;
	const float ZOOM = 45.0f;
	const glm::vec3 POSITION(0.0f, 50.0f, 0.0f);
};

class Camera {
public:
	float near = 0.1f, far = 1000.0f;

	glm::vec3 position = CAM_DEFAULTS::POSITION;
	glm::vec3 direction= glm::vec3(0.0f, 1.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 right = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	// euler Angles
	float Yaw = CAM_DEFAULTS::YAW;
	float Pitch = CAM_DEFAULTS::PITCH;
	// camera options
	float MovementSpeed =  CAM_DEFAULTS::SPEED;
	float MouseSensitivity = CAM_DEFAULTS::SENSITIVITY;
	float Zoom = CAM_DEFAULTS::ZOOM;


	Camera();


	//recalculate camera direction vector based on yaw and pitch
	void updateCameraVectors();

	glm::mat4 view();
	//view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
	//	glm::vec3(0.0f, 0.0f, 0.0f),
	//	glm::vec3(0.0f, 1.0f, 0.0f));

	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	void processSroll(float offset);

};
