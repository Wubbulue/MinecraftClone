#ifndef MINECRAFT_HEADER
#define MINECRAFT_HEADER

#include "PerlinNoise.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "geometery.h"

const int WORLD_HEIGHT = 64;
const int CHUNK_LENGTH = 64;


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

struct Block {
	BlockType type = BlockType::Air;
};

class Chunk {
public:




	siv::PerlinNoise::seed_type seed;
	//x,z,y
	Block blocks[CHUNK_LENGTH][CHUNK_LENGTH][WORLD_HEIGHT];
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

};


#endif // !MINECRAFT_HEADER
