#include "opencv2/core.hpp"
#include "minecraft.h"

void Chunk::populateBlocks() {

	const siv::PerlinNoise perlin{ seed };
	

	for (int z = 0; z < CHUNK_LENGTH; ++z)
	{
		for (int x = 0; x < CHUNK_LENGTH; ++x)
		{
			const double noise = perlin.octave2D_01((x*.01), (z*.01), 8);
			uchar pixColor = uchar(noise * CHUNK_LENGTH);
			blocks[x][z][pixColor].type = BlockType::Dirt;
			for (int y = pixColor - 1; y > -1; y--) {
				blocks[x][z][y].type = BlockType::Stone;
			}
			
		}

	}

}

void Chunk::regenerate() {
	
	seed = std::rand();
	empty();
	populateBlocks();

}

void Chunk::empty() {
	//empty block
	Block block;

	for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {
		for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
			for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
				blocks[x][z][y] = block;
			}
		}
	}
}

bool Chunk::isBlockAdjacentToAir(int x, int y, int z) {

	//must make sure we don't check outside chunk bounds

	//check x adjacency
	if ((x > 0)&&(blocks[x - 1][z][y].type == BlockType::Air)) {
		return true;
	}
	if ((x < (CHUNK_LENGTH-1))&&(blocks[x + 1][z][y].type == BlockType::Air)) {
		return true;
	}

	//check z adjacency
	if ((z > 0)&&(blocks[x][z-1][y].type == BlockType::Air)) {
		return true;
	}
	if ((z < (CHUNK_LENGTH-1))&&(blocks[x][z+1][y].type == BlockType::Air)) {
		return true;
	}

	//check y adjacency
	if ((y > 0)&&(blocks[x][z][y-1].type == BlockType::Air)) {
		return true;
	}
	if ((y < (CHUNK_HEIGHT-1))&&(blocks[x][z][y+1].type == BlockType::Air)) {
		return true;
	}


	return false;

}


void Chunk::eliminateRayIntersection(glm::vec3 rayOrigin, glm::vec3 rayVector) {

	for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {
		for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
			for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
				if (blocks[x][z][y].type != BlockType::Air) {
					Triangle blockTriangles[12];
					getBlockTriangles(x, y, z, blockTriangles);
					for (int i = 0; i < 12; i++) {
						glm::vec3 outPoint;
						if (rayIntersect(rayOrigin, rayVector, blockTriangles + i, outPoint)) {
							blocks[x][z][y].type = BlockType::Air;
							break;
						}
					}
				}
			}
		}
	}
}


void Chunk::getBlockTriangles(int x, int y, int z, Triangle triangles[12]) {

	for (int i = 0; i < 12; i++) {
		Triangle tempTri;

		//triangle offset
		short t = i*9;

		for (int j = 0; j < 3; j++) {
			tempTri.points[j].x = cubeVertices[t + j*3] + x;
			tempTri.points[j].y = cubeVertices[t + 1 + j*3] + y;
			tempTri.points[j].z = cubeVertices[t + 2 + j*3] + z;
		}

		triangles[i] = tempTri;
	}

}

void Chunk::pickBlock(glm::vec3 position) {

	bool inXRange = position.x >= 0 && position.x < CHUNK_LENGTH;
	bool inYRange = position.y >= 0 && position.y < CHUNK_HEIGHT;
	bool inZRange = position.z >= 0 && position.z < CHUNK_LENGTH;

	if (!inXRange || !inYRange || !inZRange) {
		printf("Not in range\n");
		return;
	}

	int intx = int(position.x);
	int inty = int(position.y);
	int intz = int(position.z);

	printf("Block at [%d,%d,%d] is %s\n",intx,intz,inty,blockTypeToString(blocks[intx][intz][inty].type));


}

const char* blockTypeToString(BlockType type) {
	switch (type) {

	case BlockType::Dirt:
	{
		return "Dirt";
	}

	case BlockType::Air:
	{

		return "Air";
	}

	case BlockType::Stone:
	{
		return "Stone";
	}
	}
	
}
