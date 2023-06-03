#ifndef MINECRAFT_HEADER
#define MINECRAFT_HEADER

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "PerlinNoise.h"
#include "glm/glm.hpp"
#include "geometery.h"
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <iostream>
#include "Timer.h"
#include "ThreadPool.h"
#include <cstring>

constexpr int CHUNK_HEIGHT = 64;
constexpr int CHUNK_LENGTH = 16;
#define INVALID_LIGHT_LEVEL 16

constexpr int maxNumBlockToRender = 20000;
constexpr int numFacesPerBlock = 6;
constexpr int numVertsPerFace = 6;
constexpr int numTrianglesPerFace = 2;
constexpr int numVertsPerTriangle = 3;
constexpr int numVertsPerBlock = numFacesPerBlock * numVertsPerTriangle * numTrianglesPerFace;
constexpr uint16_t renderDistance = 6;
constexpr int renderSpaceSideLength = (renderDistance * 2 + 1) * CHUNK_LENGTH;

//X,Y,Z positions, U,V tex Coord <- Float
//Face type and light level <- char
constexpr int dataPerVert = (3 + 2) * sizeof(float) + (1 + 1) * sizeof(uint8_t);

constexpr int totalDataPerBlock = (dataPerVert * numVertsPerBlock);

#define index(x,z,y) (x)+((z)*CHUNK_LENGTH)+((y)*CHUNK_LENGTH*CHUNK_LENGTH)

const float vertPositions[]{
	//negative z face
	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,

	//positive z face
	-0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,

	//negative x face
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,

	//positve x face
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,

	 //negative y face
	 -0.5f, -0.5f, -0.5f,
	  0.5f, -0.5f, -0.5f,
	  0.5f, -0.5f,  0.5f,
	  0.5f, -0.5f,  0.5f,
	 -0.5f, -0.5f,  0.5f,
	 -0.5f, -0.5f, -0.5f,

	 //positive y face
	 -0.5f,  0.5f, -0.5f,
	  0.5f,  0.5f, -0.5f,
	  0.5f,  0.5f,  0.5f,
	  0.5f,  0.5f,  0.5f,
	 -0.5f,  0.5f,  0.5f,
	 -0.5f,  0.5f, -0.5f,

};

const float texPositions[]{

	//negative z
	0.0f, 0.0f,
	0.0625f, 0.0f,
	0.0625f, 0.0625f,
	0.0625f, 0.0625f,
	0.0f, 0.0625f,
	0.0f, 0.0f,

	//positive z
	0.0f, 0.0f,
	0.0625f, 0.0f,
	0.0625f, 0.0625f,
	0.0625f, 0.0625f,
	0.0f, 0.0625f,
	0.0f, 0.0f,

	//negative x
	0.0625f, 0.0625f,
	0.0f, 0.0625f,
	0.0f, 0.0f,
	0.0f, 0.0f,
	0.0625f, 0.0f,
	0.0625f, 0.0625f,

	//positve x
	0.0f, 0.0625f,
	0.0625f, 0.0625f,
	0.0625f, 0.0f,
	0.0625f, 0.0f,
	0.0f, 0.0f,
	0.0f, 0.0625f,

	//negative y
	0.0f, 0.0625f,
	0.0625f, 0.0625f,
	0.0625f, 0.0f,
	0.0625f, 0.0f,
	0.0f, 0.0f,
	0.0f, 0.0625f,

	//positive y
	0.0f, 0.0625f,
	0.0625f, 0.0625f,
	0.0625f, 0.0f,
	0.0625f, 0.0f,
	0.0f, 0.0f,
	0.0f, 0.0625f
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
	const std::array<std::string, 4> blockTypeStrings = { "Air","Stone","Dirt","Planck" };
	const std::array<int, 4> emitsLight = {0,0,0,15};
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
		return ((b1.x == b2.x) && (b1.y == b2.y) && (b1.z == b2.z));
	}

	friend bool operator!= (const BlockPosition& b1, const BlockPosition& b2)
	{
		return ((b1.x != b2.x) || (b1.y != b2.y) || (b1.z != b2.z));
	}
};

template <>
struct std::hash<BlockPosition>
{
	std::size_t operator()(const BlockPosition& b) const
	{
		using std::hash;

		// Compute individual hash values for first,
		// second and third and combine them using XOR
		// and bit shifting:

		return ((hash<int>()(b.x)
			^ (hash<int>()(b.y) << 1)) >> 1)
			^ (hash<int>()(b.z) << 1);
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

	Chunk(int inX, int inZ) : x(inX), z(inZ) {
		blocks.resize(CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_HEIGHT);
	}



	//empties chunk
	void empty();

	bool isBlockAdjacentToAir(int x, int y, int z);

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
	void populateChunk(Chunk& chunk);

	//randomizes seed and regenerates
	void regenerate();

	World(siv::PerlinNoise::seed_type inSeed) :seed(inSeed) {

		printf("maxNumBlockToRender: %d, numVertsPerBlock: %d, dataPerVert: %d, totalDataPerBlock: %d\n", maxNumBlockToRender, numVertsPerBlock, dataPerVert, totalDataPerBlock);
		perlin = siv::PerlinNoise(seed);

		//set it to size of blocks around player
		fullWorld.resize(CHUNK_HEIGHT * renderSpaceSideLength * renderSpaceSideLength);
		airCulled.resize(fullWorld.size());
		fullCulled.resize(fullWorld.size());
		lightLevel = std::vector<uint8_t>(fullWorld.size(),0);
		//renderBuffer.resize(fullWorld.size());

		//TODO: i don't really know how big this buffer needs to be, this is a guess...
		renderBuffer.resize(5000000);
	}

	void initOpenGL();

	void cleanOpenGL() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	~World() {
		cleanOpenGL();
	}


	//traverses until a solid block is found. If none, returns false
	bool findFirstSolid(const Ray& ray, const float& length, BlockPosition& pos);

	//traverses until a solid block is found. If one is found, returns the air block right before that(where a block should be placed). If none is found, function returns false.
	bool getPlaceBlock(const Ray& ray, const float& length, BlockPosition& pos);

	uint32_t numBlocks();

	//block are inclusive at start, and non inclusive at end, except for at chunk border
	static BlockPosition findBlock(const glm::vec3& position);

	static void findChunk(const glm::vec3& position, int* chunkX, int* chunkZ);

	void removeBlock(const BlockPosition& pos);




	//gets chunk from block coords
	//returns nullptr if it can't find chunk
	Chunk* getChunkContainingBlock(const int& x, const int& z);

	Chunk* getChunkContainingPosition(const glm::vec3& position);
	Chunk* getChunkContainingBlock(const BlockPosition& pos);

	//this function tries to find a block within the worl given a block position
	Block* getBlock(const BlockPosition& pos);

	//gets chunk from chunk 
	//returns nullptr if it can't find chunk
	Chunk* getChunk(const int& x, const int& z);

	bool isBlockAdjacentToAir(BlockPosition pos);

	int customIndex(int x, int z, int y);
	int customIndex(const BlockPosition&);

	//Check if a block position in render space is inside bounds
	bool insideRenderSpace(BlockPosition& pos);

	//This function translates a position from aboslute space to render space, like full culled
	BlockPosition absoluteToRenderSpace(BlockPosition& pos);

	//This function translates a position from render space to absolute space
	BlockPosition renderSpaceToAbsoluteSpace(BlockPosition& pos);

	void propogateLight(BlockPosition &pos);
	void getBlocksToRenderThreaded(int chunkX, int chunkZ, const Frustum& camFrustum);

	//~World() {
		//for (auto& [key, chunk] : chunks)
		//{
		//	delete[] chunk.blocks;
		//}
	//}

	//this is a vector of all blocks around the player which is updated by the getBlocksToRender function to save it from being allocated every frame
	//It is organized x,z,y
	std::vector<Block> fullWorld;

	//these are the blocks that should be rendered on each frame
	std::vector<Block> airCulled;

	//these are the blocks that should be rendered on each frame
	std::vector<Block> fullCulled;

	std::vector<char> renderBuffer;


	//ranges from 0 to 15
	std::vector<uint8_t> lightLevel = { 0 };
	const uint8_t passiveLightLevel = 2;

	std::atomic_int numBlocksToRender = 0;

    uint32_t numFacesToRender = 0;

	bool renderBlocksDirty = true;
	bool frustumCullDirty = true;
	bool lightDirty = true;
	bool vboDirty = true;

	const bool alwaysRender = true;


	//number of chunks that are loaded around player, for example, distance of 4 would result in 9x9 grid of chunks
	const int worldLength = CHUNK_LENGTH * (2 * renderDistance + 1);
	void addChunk(int x, int z);

	//this function mashes our two 4 byte integers into a single 8 byte output
	//https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
	static int64_t genHash(int32_t a, int32_t b);

	//opposite of above
	static void retrieveHash(int32_t* a, int32_t* b, int64_t c);

	std::unordered_map<int64_t, Chunk> chunks;

	unsigned int getVAO() {
		return VAO;
	}



private:
	unsigned int VAO;
	unsigned int VBO;

};


#endif // !MINECRAFT_HEADER
