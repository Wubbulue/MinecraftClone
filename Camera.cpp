#include "Camera.h"

// constructor with vectors
Camera::Camera() 
{
    updateCameraVectors();
}

bool Camera::moveCamera(GLFWwindow* window, float deltaTime) {
    bool moved = false;
    const float cameraSpeed = MovementSpeed*deltaTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += cameraSpeed * direction;
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= cameraSpeed * direction;
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        moved = true;
        position -= glm::normalize(glm::cross(direction, up)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        moved = true;
        position += glm::normalize(glm::cross(direction, up)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        moved = true;
        position.y += cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        moved = true;
        position.y -= cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        moved = true;
        position.y -= cameraSpeed;
    }

    updateCameraVectors();
    return moved;

}

glm::mat4 Camera::view() {
    return glm::lookAt(position, position + direction, up);
}

void Camera::updateCameraVectors() {
    glm::vec3 temp;
    temp.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    temp.y = sin(glm::radians(Pitch));
    temp.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

    direction = glm::normalize(temp);
    right = glm::normalize(glm::cross(direction, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    up = glm::normalize(glm::cross(right, direction));

}
// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}


void Camera::processSroll(float offset) {
	Zoom -= offset;
	if (Zoom < 1.0f)
		Zoom = 1.0f;
	if (Zoom > 45.0f)
		Zoom = 45.0f;
}
