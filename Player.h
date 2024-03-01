#pragma once

#include "minecraft.h"
#include "Camera.h"
#include "glm/glm.hpp"

class Player {
public:


	Player(World* world) : world(world)
	{}

	//TODO: maybe put camera inside player, WARNING, adding camera header to this file caused linker errors for some reason
	Camera cam;

	//this is the block the player is looking at 
	BlockPosition blockLookingAt;

	//if the player is looking at a block, this is true
	bool isLookingAtBlock;

	int chunkX = 0, chunkZ = 0;


	//Based on input state from the window, computes movement velocity
	//void computeMovementVelocity(GLFWwindow* window);

	void setPosition(const glm::vec3& inPos);

	//Moves player based on velocity and delta time
	bool tick(float deltaTime,GLFWwindow* window);

	bool walkMode = false;


	void toggleMovementMode();


private:

	World* world;

	//Time it takes to fully interpolate from keypress, in seconds
	const float full_keypress_time = 0.25f;
	float w_ratio = 0.0f;
	float s_ratio = 0.0f;
	float a_ratio = 0.0f;
	float d_ratio = 0.0f;
	float space_ratio = 0.0f;
	float shift_ratio = 0.0f;

	//This position exists at the player's head
	glm::vec3 position = CAM_DEFAULTS::POSITION;


	//this is velocity originated from player input, like wasd
	//glm::vec3 movement_velocity = glm::vec3(0.0f, 0.0f, 0.0f);

	//this function calculates chunkx and chunkZ for the player
	void recalcChunkPos();

	const float FLY_SPEED = 30.0f;
	const float WALK_SPEED = 10.0f;
	float MOVEMENT_SPEED = FLY_SPEED;

	const float gravityAcceleration = -9.8f;
	glm::vec3 physics_velocity = glm::vec3(0.0f,0.0f,0.0f);


};

