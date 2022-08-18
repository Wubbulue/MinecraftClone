#ifndef MINECRAFT_HEADER
#define MINECRAFT_HEADER

#include "PerlinNoise.h"
#include "glm/glm.hpp"
#include "geometery.h"
#include "Logging.h"
#include <limits>
#include <unordered_map>
#include <array>
#include <iostream>
#include "Timer.h"
#include "ThreadPool.h"

const int CHUNK_HEIGHT = 64;
const int CHUNK_LENGTH = 16;

#define index(x,z,y) (x)+((z)*CHUNK_LENGTH)+((y)*CHUNK_LENGTH*CHUNK_LENGTH)


const float cubeVertices[]{
	//negative z
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  0.0625f, 0.0f,
	 0.5f,  0.5f, -0.5f,  0.0625f, 0.0625f,
	 0.5f,  0.5f, -0.5f,  0.0625f, 0.0625f,
	-0.5f,  0.5f, -0.5f,  0.0f, 0.0625f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	//positive z
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0625f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.0625f, 0.0625f,
	 0.5f,  0.5f,  0.5f,  0.0625f, 0.0625f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0625f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	//negative x
	-0.5f,  0.5f,  0.5f,  0.0625f, 0.0625f,
	-0.5f,  0.5f, -0.5f,  0.0f, 0.0625f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0625f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0625f, 0.0625f,

	//positve x
	 0.5f,  0.5f,  0.5f,  0.0f, 0.0625f,
	 0.5f,  0.5f, -0.5f,  0.0625f, 0.0625f,
	 0.5f, -0.5f, -0.5f,  0.0625f, 0.0f,
	 0.5f, -0.5f, -0.5f,  0.0625f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f, 0.0625f,

	//negative y
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0625f,
	 0.5f, -0.5f, -0.5f,  0.0625f, 0.0625f,
	 0.5f, -0.5f,  0.5f,  0.0625f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0625f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0625f,

	//positive y
	-0.5f,  0.5f, -0.5f,  0.0f, 0.0625f,
	 0.5f,  0.5f, -0.5f,  0.0625f, 0.0625f,
	 0.5f,  0.5f,  0.5f,  0.0625f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.0625f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 0.0625f
};


//TODO: this is clearly really bad...
const uint8_t dirtFaces[]{
	3,3,3,3,3,3,
	3,3,3,3,3,3,
	3,3,3,3,3,3,
	3,3,3,3,3,3,
	2,2,2,2,2,2,
	0,0,0,0,0,0,
};

const uint8_t stoneFaces[]{
	1,1,1,1,1,1,
	1,1,1,1,1,1,
	1,1,1,1,1,1,
	1,1,1,1,1,1,
	1,1,1,1,1,1,
	1,1,1,1,1,1,
};

const uint8_t planckFaces[]{
	4,4,4,4,4,4,
	4,4,4,4,4,4,
	4,4,4,4,4,4,
	4,4,4,4,4,4,
	4,4,4,4,4,4,
	4,4,4,4,4,4,
};


typedef uint8_t BlockType;

//have to do this weird hacky thing because enums are 4 bytes long when we only need 1 byte for block type
namespace BlockTypes {
	const BlockType Air = 0;
	const BlockType Stone = 1;
	const BlockType Dirt = 2;
	const BlockType Planck = 3;
	const std::array<std::string, 4> blockTypeStrings = {"Air","Stone","Dirt","Planck"};
	std::string blockTypeToString(BlockType type);
};

//enum class BlockType {
//	Dirt,
//	Air,
//	Stone
//};


struct Block {
	BlockType type = BlockTypes::Air;
};

const int chunkDataOffset = CHUNK_HEIGHT * CHUNK_LENGTH * CHUNK_LENGTH * sizeof(Block);

struct BlockPosition {
	int x, y, z;

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
	int x, z;



	//this will copy heap memory every time
	//TODO: make this better
	std::vector<Block> blocks;

	//this function returns a block given a block position that is global
	Block* indexAbsolute(BlockPosition pos);

	Chunk(int inX,int inZ) : x(inX), z(inZ) {
		blocks.resize(CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_HEIGHT);
	}



	//empties chunk
	void empty();

	bool isBlockAdjacentToAir(int x, int y, int z);

	void eliminateRayIntersection(glm::vec3 rayOrigin, glm::vec3 rayVector);

	//generates a list of the triangle geometery for a certain block
	void getBlockTriangles(int x, int y, int z, Triangle triangles[12]);

	//finds a block and returns its position as x,y,z
	//block are inclusive at start, and non inclusive at end, except for at chunk border
	BlockPosition findBlock(glm::vec3 position);




	Box3 getBox();


};

class World {
public:

	siv::PerlinNoise::seed_type seed;
	siv::PerlinNoise perlin;

	void populateChunks();
	void populateChunk(Chunk &chunk);

	//randomizes seed and regenerates
	void regenerate();

	World(siv::PerlinNoise::seed_type inSeed) :seed(inSeed) {
		perlin = siv::PerlinNoise(seed);

		//set it to size of blocks around player
		fullWorld.resize(CHUNK_HEIGHT*CHUNK_LENGTH*CHUNK_LENGTH*(pow(renderDistance*2+1,2)));
		airCulled.resize(fullWorld.size());
		fullCulled.resize(fullWorld.size());
		lightLevel.resize(fullWorld.size());

	}

	//will only currently work for rays cast inside world bounds 
	void mineHoleCast(const Ray& ray, const float& length);

	//traverses until a solid block is found. If none, returns false
	bool findFirstSolid(const Ray& ray, const float& length, BlockPosition& pos);

	//traverses until a solid block is found. If one is found, returns the air block right before that(where a block should be placed). If none is found, function returns false.
	bool getPlaceBlock(const Ray& ray, const float& length, BlockPosition& pos);

	uint32_t numBlocks();

	//block are inclusive at start, and non inclusive at end, except for at chunk border
	static BlockPosition findBlock(const glm::vec3 &position);
	
	static void findChunk(const glm::vec3 &position, int *chunkX,int *chunkZ);

	void removeBlock(const BlockPosition& pos);

	


	//gets chunk from block coords
	//returns nullptr if it can't find chunk
	Chunk* getChunkContainingBlock(const int& x,const int& z);

	Chunk* getChunkContainingPosition(const glm::vec3 &position);
	Chunk* getChunkContainingBlock(const BlockPosition& pos);

	//this function tries to find a block within the worl given a block position
	Block* getBlock(const BlockPosition &pos);

	//gets chunk from chunk 
	//returns nullptr if it can't find chunk
	Chunk* getChunk(const int& x,const int& z);

	bool isBlockAdjacentToAir(BlockPosition pos);

	int customIndex(int x, int z, int y);
	int customIndex(const BlockPosition &);


	void getBlocksToRenderThreaded(int chunkX,int chunkZ, const Frustum &camFrustum);

	//~World() {
		//for (auto& [key, chunk] : chunks)
		//{
		//	delete[] chunk.blocks;
		//}
	//}

	//this is a vector of all blocks around the player which is updated by the getBlocksToRender function to save it from being allocated every frame
	std::vector<Block> fullWorld;

	//these are the blocks that should be rendered on each frame
	std::vector<Block> airCulled;

	//these are the blocks that should be rendered on each frame
	std::vector<Block> fullCulled;


	//ranges from 0 to 15
	std::vector<uint8_t> lightLevel = {0};

	bool renderBlocksDirty = true;
	

	//number of chunks that are loaded around player, for example, distance of 4 would result in 9x9 grid of chunks
	const uint16_t renderDistance = 6;
	const int worldLength = CHUNK_LENGTH * (2 * renderDistance + 1);
	void addChunk(int x, int z);

	//this function mashes our two 4 byte integers into a single 8 byte output
	//https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
	static int64_t genHash(int32_t a, int32_t b);

	//opposite of above
	static void retrieveHash(int32_t *a, int32_t *b, int64_t c);

	std::unordered_map<int64_t, Chunk> chunks;
	


private:
	unsigned int vao;
	unsigned int vbo;

};


#endif // !MINECRAFT_HEADER
