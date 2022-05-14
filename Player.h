#pragma once

#include "minecraft.h"
#include "Camera.h"
#include "glm/glm.hpp"

class Player {
public:


	//TODO: maybe put camera inside player, WARNING, adding camera header to this file caused linker errors for some reason
	Camera cam;

	//this is the block the player is looking at 
	BlockPosition blockLookingAt;

	//if the player is looking at a block, this is true
	bool isLookingAtBlock;

	int chunkX = 0, chunkZ = 0;

	bool checkPosition(GLFWwindow* window,float deltaTime);

	void movePlayer(const glm::vec3 &inPos);

	float MovementSpeed =  10.0f;

private:
	glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f);

	//this function calculates chunkx and chunkZ for the player
	void recalcChunkPos();

};

