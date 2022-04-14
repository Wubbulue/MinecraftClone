#pragma once

#include "minecraft.h"

class Player {
public:
	//TODO: maybe put camera inside player, WARNING, adding camera header to this file caused linker errors for some reason

	//this is the block the player is looking at 
	BlockPosition blockLookingAt;

	//if the player is looking at a block, this is true
	bool isLookingAtBlock;
};

