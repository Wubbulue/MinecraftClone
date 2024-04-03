#include "Player.h"

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

void Player::toggleMovementMode() {
    walkMode = !walkMode;
    physics_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    MOVEMENT_SPEED = walkMode ? WALK_SPEED : FLY_SPEED;
}

bool Player::tick(float deltaTime,GLFWwindow* window) {

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        w_ratio += deltaTime/full_keypress_time;
    }
    else {
        w_ratio -= deltaTime/full_keypress_time;
    }
    w_ratio = glm::clamp(w_ratio, 0.0f, 1.0f);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        s_ratio += deltaTime/full_keypress_time;
    }
    else {
        s_ratio -= deltaTime/full_keypress_time;
    }
    s_ratio = glm::clamp(s_ratio, 0.0f, 1.0f);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        a_ratio += deltaTime/full_keypress_time;
    }
    else {
        a_ratio -= deltaTime/full_keypress_time;
    }
    a_ratio = glm::clamp(a_ratio, 0.0f, 1.0f);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        d_ratio += deltaTime/full_keypress_time;
    }
    else {
        d_ratio -= deltaTime/full_keypress_time;
    }
    d_ratio = glm::clamp(d_ratio, 0.0f, 1.0f);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        space_ratio += deltaTime/full_keypress_time;
    }
    else {
        space_ratio -= deltaTime/full_keypress_time;
    }
    space_ratio = glm::clamp(space_ratio, 0.0f, 1.0f);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        shift_ratio += deltaTime/full_keypress_time;
    }
    else {
        shift_ratio -= deltaTime/full_keypress_time;
    }
    shift_ratio = glm::clamp(shift_ratio, 0.0f, 1.0f);

	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

	velocity += cam.direction*MOVEMENT_SPEED*w_ratio;

	velocity -= cam.direction*MOVEMENT_SPEED*s_ratio;

	velocity -= glm::normalize(glm::cross(cam.direction, cam.up))*MOVEMENT_SPEED*a_ratio;

	velocity += glm::normalize(glm::cross(cam.direction, cam.up))*MOVEMENT_SPEED*d_ratio;

	velocity += cam.worldUp*MOVEMENT_SPEED*space_ratio;

    if (!walkMode) {
		velocity -= cam.worldUp*MOVEMENT_SPEED*shift_ratio;
    }
    else {
        physics_velocity.y += gravityAcceleration*deltaTime;
        velocity += physics_velocity;
    }

    




    const glm::vec3 posDifference = velocity * deltaTime; // adjust accordingly
    if (posDifference == glm::vec3(0.0f, 0.0f, 0.0f)) {
        return false;
    }
    position += posDifference;
    if (!walkMode) {
		BlockPosition blockPos = World::vectorToBlockPosition(position);
		if (world->getBlock(blockPos)->type != BlockTypes::Air) {
			position -= posDifference;
		}
    }
    cam.position = position;
    recalcChunkPos();
    return true;
}
