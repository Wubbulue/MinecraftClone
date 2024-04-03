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

constexpr int maxNumBlockToRender = 20000;
constexpr int numFacesPerBlock = 6;
constexpr int numVertsPerFace = 6;
constexpr int numTrianglesPerFace = 2;
constexpr int numVertsPerTriangle = 3;
constexpr int numVertsPerBlock = numFacesPerBlock * numVertsPerTriangle * numTrianglesPerFace;
constexpr uint16_t renderDistance = 6;
constexpr int renderSpaceSideLength = (renderDistance * 2 + 1) * CHUNK_LENGTH;

//X,Y,Z positions, U,V tex Coord <- Float
//Face type <- char
//packed light <- uint
constexpr int dataPerVert = (3 + 2) * sizeof(float) + (1) * sizeof(uint8_t) + (1) * sizeof(uint32_t);

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


//TODO: this is clearly really bad, need to abstract this
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

const uint8_t whiteFaces[]{
	225,225,225,225,225,225,
	225,225,225,225,225,225,
	225,225,225,225,225,225,
	225,225,225,225,225,225,
	225,225,225,225,225,225,
	225,225,225,225,225,225,
};

const uint8_t redFaces[]{
	129,129,129,129,129,129,
	129,129,129,129,129,129,
	129,129,129,129,129,129,
	129,129,129,129,129,129,
	129,129,129,129,129,129,
	129,129,129,129,129,129,
};

const uint8_t greenFaces[]{
	145,145,145,145,145,145,
	145,145,145,145,145,145,
	145,145,145,145,145,145,
	145,145,145,145,145,145,
	145,145,145,145,145,145,
	145,145,145,145,145,145,
};


typedef uint8_t BlockType;

typedef uint32_t colorPacked;
struct colorUnpacked{

	//Each of these is only 4 bits (0 - 15)
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t intensity;

	bool operator==(const colorUnpacked& r)
	{

		return red == r.red && green == r.green && blue == r.blue && intensity == r.intensity;
	}
};
constexpr colorPacked INVALID_LIGHT_LEVEL = 0xFFFFFFFF;

namespace BlockTypes {
	const BlockType Air = 0;
	const BlockType Stone = 1;
	const BlockType Dirt = 2;
	const BlockType Planck = 3;
	const BlockType whiteLight = 4;
	const BlockType redLight = 5;
	const BlockType greenLight = 6;
	const std::array<std::string, 7> blockTypeStrings = { "Air","Stone","Dirt","Planck","White Light","Red Light", "Green Light"};
	const colorUnpacked noLight = { 0,0,0,0 };
	const colorUnpacked white= { 15,15,15,15 };
	const colorUnpacked red= { 15,0,0,15 };
	const colorUnpacked green= { 0,15,0,15 };
	const std::array<colorUnpacked, 7> unpackedLights = { noLight,noLight,noLight,noLight,white,red,green};
	const std::array<const uint8_t*, 7> facesArray = {nullptr,stoneFaces,dirtFaces,planckFaces,whiteFaces,redFaces,greenFaces};
	std::string blockTypeToString(BlockType type);
};

bool emitsLight(const BlockType type);

const uint8_t* getFacesArray(const BlockType type);



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

//Implement hash function for block position so we can put them in a hashed set
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



	//Actual set of blocks within the chunk
	std::vector<Block> blocks;

	//this function returns a block given a block position that is global
	Block* indexAbsolute(BlockPosition pos);

	Chunk(int inX, int inZ) : x(inX), z(inZ) {
		//Allocate memory for our block buffer upopn construction
		blocks.resize(CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_HEIGHT);
	}



	//empties chunk
	void empty();

	bool isBlockAdjacentToAir(int x, int y, int z);

	//finds a block and returns its position as x,y,z
	//block are inclusive at start, and non inclusive at end, except for at chunk border
	BlockPosition vectorToBlockPosition(glm::vec3 position);


	//Gets a box3 surrounding the chunk, used for frustum culling
	Box3 getBox();
};


class World {
public:

	//Use bitshifting to pack an unpacked color into a packed one
    static colorPacked packColor(const colorUnpacked color) {
		colorPacked packed = 0x0000;

		packed |= (color.red & 0xFF) << 24;
		packed |= (color.green & 0xFF) << 16;
		packed |= (color.blue & 0xFF) << 8;
		packed |= color.intensity & 0xFF;

		return packed;
	}

	//Use bitshifting to unpack a packed color
	static colorUnpacked unpackColor(const colorPacked color) {
		colorUnpacked unpacked = {0,0,0,0};
		unpacked.red |= (color >> 24)&0xFF;
		unpacked.green |= (color  >> 16)&0xFF;
		unpacked.blue |= (color  >> 8)&0xFF;
		unpacked.intensity |= color&0xFF;
		return unpacked;
	}

	//Perlin noise generator used for world generation
	siv::PerlinNoise::seed_type seed;
	siv::PerlinNoise perlin;

	//Generate and fill all chunks
	void populateChunks();

	//Generate and fill a single chunk
	void populateChunk(Chunk& chunk);

	//randomizes seed and regenerates
	void regenerate();

	World(siv::PerlinNoise::seed_type inSeed) :seed(inSeed) {

		printf("maxNumBlockToRender: %d, numVertsPerBlock: %d, dataPerVert: %d, totalDataPerBlock: %d\n", maxNumBlockToRender, numVertsPerBlock, dataPerVert, totalDataPerBlock);

		//Seed our noise generator
		perlin = siv::PerlinNoise(seed);

		//Allocate all of our persistent buffers
		fullWorld.resize(CHUNK_HEIGHT * renderSpaceSideLength * renderSpaceSideLength);
		airCulled.resize(fullWorld.size());
		fullCulled.resize(fullWorld.size());
		unpackedLight = std::vector<colorUnpacked>(fullWorld.size(),ambientLight);
		packedLight = std::vector<colorPacked>(fullWorld.size(),0);

		//TODO: i don't really know how big this buffer needs to be, this is a guess...
		renderBuffer.resize(5000000);
	}

	void initOpenGL();

	void cleanOpenGL() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	//~World() {
	//	cleanOpenGL();
	//}


	//traverses until a solid block is found. If none, returns false
	bool findFirstSolid(const Ray& ray, const float& length, BlockPosition& pos);

	//traverses until a solid block is found. If one is found, returns the air block right before that(where a block should be placed). If none is found, function returns false.
	bool getPlaceBlock(const Ray& ray, const float& length, BlockPosition& pos);

	//Count the number of blocks in the world
	uint32_t numBlocks();

	//Convert a vector world position to a rounded block position
	//block are inclusive at start, and non inclusive at end, except for at chunk border
	static BlockPosition vectorToBlockPosition(const glm::vec3& position);


	//Given a vector position, finds the chunkx and chunkz that the position lives in
	static void findChunk(const glm::vec3& position, int* chunkX, int* chunkZ);

	//"Delete" a block from the world by converting it to air
	void removeBlock(const BlockPosition& pos);

	//gets chunk from block coords
	//returns nullptr if it can't find chunk
	Chunk* getChunkContainingBlock(const int& x, const int& z);

	Chunk* getChunkContainingPosition(const glm::vec3& position);
	Chunk* getChunkContainingBlock(const BlockPosition& pos);

	//this function tries to find a block within the world given a block position
	//Returns nullptr if it can't find it
	Block* getBlock(const BlockPosition& pos);

	//gets chunk from chunk 
	//returns nullptr if it can't find chunk
	Chunk* getChunk(const int& x, const int& z);

	//Checks if block is adjacent to an air block in any four directions (used for rendering)
	bool isBlockAdjacentToAir(BlockPosition pos);

	//Convert a 3d position into a 1d index we can access buffer data with
	int customIndex(int x, int z, int y);
	int customIndex(const BlockPosition&);

	//Check if a block position in render space is inside bounds
	bool insideRenderSpace(BlockPosition& pos);

	//This function translates a position from aboslute space to render space, like full culled
	BlockPosition absoluteToRenderSpace(BlockPosition& pos);

	//This function translates a position from render space to absolute space
	BlockPosition renderSpaceToAbsoluteSpace(BlockPosition& pos);

	//Blend two light colors together, seeems to not be working completely?
	static void blendLight(const colorUnpacked *src,colorUnpacked *dest);


	//Given a block position that has a light at it, use flood fill algorithm to spread the light
	void propogateLight(BlockPosition &pos);

	//Given the players chunk x and chunk z along with the camera frustum, render the world
	void renderThreaded(int chunkX, int chunkZ, const Frustum& camFrustum);


	//this is a vector of all blocks around the player which is updated by the getBlocksToRender function to save it from being allocated every frame
	//It is organized x,z,y
	std::vector<Block> fullWorld;

	//these are the blocks that should be rendered on each frame
	std::vector<Block> airCulled;

	//these are the blocks that should be rendered on each frame
	std::vector<Block> fullCulled;

	//Render buffer bytes to be sent to opengl
	std::vector<char> renderBuffer;


	//ranges from 0 to 15
	std::vector<colorUnpacked> unpackedLight = { {1,0,0,0} };
	std::vector<colorPacked> packedLight = {0};

	colorUnpacked ambientLight = { 15,15,15,5 };

	std::atomic_int numBlocksToRender = 0;

    uint32_t numFacesToRender = 0;

	//A set of flags used to determine which stages in the render process are dirty
	bool renderBlocksDirty = true;
	bool frustumCullDirty = true;
	bool lightDirty = true;
	bool vboDirty = true;

	const bool alwaysRender = false;


	//number of chunks that are loaded around player, for example, distance of 4 would result in 9x9 grid of chunks
	const int worldLength = CHUNK_LENGTH * (2 * renderDistance + 1);
	void addChunk(int x, int z);

	//this function mashes our two 4 byte integers into a single 8 byte output
	//https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
	//Given a chunk x and z, hashses them together
	static int64_t genHash(int32_t a, int32_t b);

	//opposite of above
	static void retrieveHash(int32_t* a, int32_t* b, int64_t c);

	//Given a chunk position hash key, returns corresponding chunk
	std::unordered_map<int64_t, Chunk> chunks;

	unsigned int getVAO() {
		return VAO;
	}



private:
	unsigned int VAO;
	unsigned int VBO;

};


#endif // !MINECRAFT_HEADER
