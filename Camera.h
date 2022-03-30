#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
	bool fpsMode = true;
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 direction= glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 right = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	// euler Angles
	float Yaw = YAW;
	float Pitch = PITCH;
	// camera options
	float MovementSpeed =  SPEED;
	float MouseSensitivity = SENSITIVITY;
	float Zoom = ZOOM;


	Camera();


	//check user input and move camera accordingly
	void moveCamera(GLFWwindow* window,float deltaTime);

	//recalculate camera direction vector based on yaw and pitch
	void updateCameraVectors();

	glm::mat4 view();
	//view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
	//	glm::vec3(0.0f, 0.0f, 0.0f),
	//	glm::vec3(0.0f, 1.0f, 0.0f));

	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	void processSroll(float offset);

};
