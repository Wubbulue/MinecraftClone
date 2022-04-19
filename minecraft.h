#ifndef MINECRAFT_HEADER
#define MINECRAFT_HEADER

#include "PerlinNoise.h"
#include "glm/glm.hpp"
#include "geometery.h"
#include <cassert>
#include <limits>
#include <unordered_map>
#include <array>

const int CHUNK_HEIGHT = 64;
const int CHUNK_LENGTH = 64;

#define index(x,z,y) x*z*y


const float cubeVertices[] = {
-0.5f, -0.5f, -0.5f,
 0.5f, -0.5f, -0.5f,
 0.5f,  0.5f, -0.5f,
 0.5f,  0.5f, -0.5f,
-0.5f,  0.5f, -0.5f,
-0.5f, -0.5f, -0.5f,

-0.5f, -0.5f,  0.5f,
 0.5f, -0.5f,  0.5f,
 0.5f,  0.5f,  0.5f,
 0.5f,  0.5f,  0.5f,
-0.5f,  0.5f,  0.5f,
-0.5f, -0.5f,  0.5f,

-0.5f,  0.5f,  0.5f,
-0.5f,  0.5f, -0.5f,
-0.5f, -0.5f, -0.5f,
-0.5f, -0.5f, -0.5f,
-0.5f, -0.5f,  0.5f,
-0.5f,  0.5f,  0.5f,

 0.5f,  0.5f,  0.5f,
 0.5f,  0.5f, -0.5f,
 0.5f, -0.5f, -0.5f,
 0.5f, -0.5f, -0.5f,
 0.5f, -0.5f,  0.5f,
 0.5f,  0.5f,  0.5f,

-0.5f, -0.5f, -0.5f,
 0.5f, -0.5f, -0.5f,
 0.5f, -0.5f,  0.5f,
 0.5f, -0.5f,  0.5f,
-0.5f, -0.5f,  0.5f,
-0.5f, -0.5f, -0.5f,

-0.5f,  0.5f, -0.5f,
 0.5f,  0.5f, -0.5f,
 0.5f,  0.5f,  0.5f,
 0.5f,  0.5f,  0.5f,
-0.5f,  0.5f,  0.5f,
-0.5f,  0.5f, -0.5f,
};


enum class BlockType {
	Dirt,
	Air,
	Stone
};

const char* blockTypeToString(BlockType type);

struct Block {
	BlockType type = BlockType::Air;
};

struct BlockPosition {
	uint16_t x, y, z;

	friend bool operator== (const BlockPosition& b1, const BlockPosition& b2)
	{
		return ( (b1.x == b2.x) && (b1.y==b2.y) && (b1.z==b2.z) );
	}

	friend bool operator!= (const BlockPosition& b1, const BlockPosition& b2)
	{
		return ( (b1.x != b2.x) || (b1.y!=b2.y) || (b1.z!=b2.z) );
	}
};

class Chunk {
public:

	//these variables define chunk's position within the world
	int x = 0, z=0;



	siv::PerlinNoise::seed_type seed;
	//x,z,y
	Block blocks[CHUNK_LENGTH][CHUNK_LENGTH][CHUNK_HEIGHT];
	//Block* blocks = new Block[CHUNK_LENGTH*CHUNK_LENGTH*CHUNK_HEIGHT];

	
	Chunk(siv::PerlinNoise::seed_type inSeed) :seed(inSeed) {
		populateBlocks();
	}
	void populateBlocks();

	//randomizes seed and regenerates
	void regenerate();

	//empties chunk
	void empty();

	bool isBlockAdjacentToAir(int x, int y, int z);

	void eliminateRayIntersection(glm::vec3 rayOrigin, glm::vec3 rayVector);

	//generates a list of the triangle geometery for a certain block
	void getBlockTriangles(int x, int y, int z, Triangle triangles[12]);

	//finds a block and returns its position as x,y,z
	//block are inclusive at start, and non inclusive at end, except for at chunk border
	BlockPosition findBlock(glm::vec3 position);

	//mines hole along ray
	void mineHoleCast(const Ray& ray);

	//traverses until a solid block is found. If none, returns false
	bool findFirstSolid(const Ray& ray, BlockPosition& pos);

	Box3 getBox();


};

class World {
public:
	//number of chunks that are loaded around player, for example, distance of 4 would result in 9x9 grid of chunks
	uint16_t renderDistance = 4;
	void addChunk(int x, int z, Chunk chunk);

	//this function hashes our two integers into a unique output
	static long cantorHash(int a, int b);
	


private:
	std::unordered_map<long, Chunk> chunks;

};


#endif // !MINECRAFT_HEADER
