#include "glm/glm.hpp"
#include "minecraft.h"
#include "Player.h"
#include <string>
#include <iostream>
#include <fstream>
#include "Camera.h"
#include <vector>
#include <filesystem>
#include <iterator>
#include <cstring>
#include <utility>
#include <set>
#include <cstdio>

typedef float precision;
//data offset for camera, three floats in position vector, and 2 floats for yaw and pitch
const int cameraDataOffset = sizeof(precision) * 5;
const bool overwriteFile = true;

//data offset for one chunk

//World file format:
/*

Cam Position.x (float, 4 bytes)
Cam Position.y (float, 4 bytes)
Cam Position.z (float, 4 bytes)

Cam Yaw (float, 4 bytes)
Cam Pitch (float, 4 bytes)

Number of Chunks (uint32_t, 4 bytes)

Chunk cantor hashes (int64_t, 8 bytes*number of chunks)

Chunk data (uint8_t, 1 byte*number of chunks*CHUNK_WIDTH*CHUNK_WIDTH*CHUNK_HEIGHT)


*/
class worldSaver{


public:
	std::string filename;

	Player* player;


	worldSaver(const std::string& inFile, Player *inPlayer);

	void closeWorld();

	void writePosition();

	//this method recives a chunk and fills it if it exists in world file, otherwise it returns false
	bool tryFillChunk(Chunk *chunk);

	//this method writes a chunk to the world file
	//if it already exists in the world file, then it is found and updated, otherwise it is added
	void writeChunk(const Chunk &chunk);

	//this function takes the blocks from a chunk and writes them
	void rawChunkWrite(const Chunk& chunk);

private:
	//stores the hash of chunks that we have made so far
	std::vector<int64_t> chunkList;
	std::fstream file;
};


//this class exists to load new chunks from world file based on player position updates
class ChunkManager {

public:
	ChunkManager(worldSaver* inSaver, Player* inPlayer, World* inWorld):saver(inSaver),player(inPlayer),world(inWorld)
	{
		oldChunkX = player->chunkX;
		oldChunkZ = player->chunkZ;
	}

	//call this function after construction to load to necessary chunks based on player posiiton
	void initWorld();

	//this function should be called everytime that the player moves, to check if he is in a new chunk
	//if he is, then unload the old irrelevant chunks and load new ones based on render distance
	void checkNewChunk();

	void loadChunk(const int& chunkX, const int& chunkZ);
	void unloadChunk(const int& chunkX, const int& chunkZ);

	//overload that allows input of hash
	void unloadChunk(const int64_t& hash);


private:
	worldSaver* saver;
	Player* player;
	World* world;
	//in the event of a player position update, these store the last chunk that the player was in
	int oldChunkX, oldChunkZ;

};


