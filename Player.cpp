#include "Player.h"


void Player::computeMovementVelocity(GLFWwindow* window) {

    //Start with zero velocity vector
    movement_velocity = glm::vec3(0.0f, 0.0f, 0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        movement_velocity += cam.direction*FLY_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        movement_velocity -= cam.direction*FLY_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        movement_velocity -= glm::normalize(glm::cross(cam.direction, cam.up))*FLY_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        movement_velocity += glm::normalize(glm::cross(cam.direction, cam.up))*FLY_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        movement_velocity += cam.worldUp*FLY_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        movement_velocity -= cam.worldUp*FLY_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        movement_velocity -= cam.worldUp*FLY_SPEED;
    }

}


void Player::setPosition(const glm::vec3& inPos)
{
	position = inPos;
	cam.position = position;
	recalcChunkPos();
}
void Player::recalcChunkPos()
{
	World::findChunk(position, &chunkX, &chunkZ);
}

bool Player::tick(float deltaTime) {
    if (walkMode) {
        physics_velocity += gravity * deltaTime;
    }
    const glm::vec3 posDifference = (physics_velocity+movement_velocity) * deltaTime; // adjust accordingly
    if (posDifference == glm::vec3(0.0f, 0.0f, 0.0f)) {
        return false;
    }
    position += posDifference;
    cam.position = position;
    recalcChunkPos();
    return true;
}
