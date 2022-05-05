#include "glm/glm.hpp"
#include "minecraft.h"
#include <string>
#include <iostream>
#include <fstream>
#include "Camera.h"
#include <vector>
#include <filesystem>
#include <iterator>
#include <cstring>

typedef float precision;
//data offset for camera, three floats in position vector, and 2 floats for yaw and pitch
const int cameraDataOffset = sizeof(precision) * 5;

//data offset for one chunk
const int chunkDataOffset = CHUNK_HEIGHT * CHUNK_HEIGHT * CHUNK_LENGTH * sizeof(Block);

//World file format:
/*

Cam Position.x (float, 4 bytes)
Cam Position.y (float, 4 bytes)
Cam Position.z (float, 4 bytes)

Cam Yaw (float, 4 bytes)
Cam Pitch (float, 4 bytes)

Number of Chunks (uint32_t, 4 bytes)

Chunk cantor hashes (long, 4 bytes*number of chunks)

Chunk data (uint8_t, 1 byte*number of chunks*CHUNK_WIDTH*CHUNK_WIDTH*CHUNK_HEIGHT)


*/
class worldSaver{


public:
	std::string filename;

	Camera* cam;


	worldSaver(const std::string& inFile, Camera *inCam);

	void closeWorld();

	void writePosition();

	//this method recives a chunk and fills it if it exists in world file, otherwise it returns false
	bool tryFillChunk(Chunk *chunk);

	//this method writes a chunk to the world file
	//if it already exists in the world file, then it is found and updated, otherwise it is added
	void writeChunk(const Chunk &chunk);

private:
	//stores the cantor hash of chunks that we have made so far
	std::vector<long> chunkList;
	std::fstream file;


};

bool loadWorld(const std::string &filename,World &world,Camera &cam);

