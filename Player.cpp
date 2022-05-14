#include "Player.h"


bool Player::checkPosition(GLFWwindow* window, float deltaTime)
{

	auto moved = cam.moveCamera(window, deltaTime);
	if (moved) {
		position = cam.position;
		recalcChunkPos();
	}

	return moved;

}

void Player::movePlayer(const glm::vec3& inPos)
{
	position = inPos;
	cam.position = position;
	recalcChunkPos();
}

void Player::recalcChunkPos()
{
	World::findChunk(position, &chunkX, &chunkZ);
}
