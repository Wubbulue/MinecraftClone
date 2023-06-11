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


	//Based on input state from the window, computes movement velocity
	void computeMovementVelocity(GLFWwindow* window);

	void setPosition(const glm::vec3& inPos);

	//Moves player based on velocity and delta time
	bool tick(float deltaTime);

	bool walkMode = false;

	//this is velocity coming from the world, like gravity or explosion
	glm::vec3 physics_velocity = glm::vec3(0.0f, 0.0f, 0.0f);

private:
	//This position exists at the player's head
	glm::vec3 position = CAM_DEFAULTS::POSITION;


	//this is velocity originated from player input, like wasd
	glm::vec3 movement_velocity = glm::vec3(0.0f, 0.0f, 0.0f);

	//this function calculates chunkx and chunkZ for the player
	void recalcChunkPos();

	const float FLY_SPEED = 30.0f;

	const glm::vec3 gravity = glm::vec3(0.0f,-9.8f,0.0f);


};

