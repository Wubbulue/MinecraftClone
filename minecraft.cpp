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
			for (unsigned int y = 0; y < WORLD_HEIGHT; y++) {
				blocks[x][z][y] = block;
			}
		}
	}
}

bool Chunk::isBlockAdjacentToAir(int x, int y, int z) {


	if (x > 0) {
		if ((blocks[x + 1][z][y].type == BlockType::Air)||(blocks[x - 1][z][y].type == BlockType::Air)) {
			return true;
		}
	}
	else {
		if ((blocks[x + 1][z][y].type == BlockType::Air)) {
			return true;
		}
	}

	if (y > 0) {
		if ((blocks[x][z][y+1].type == BlockType::Air)||(blocks[x][z][y-1].type == BlockType::Air)) {
			return true;
		}
	}
	else {
		if ((blocks[x][z][y+1].type == BlockType::Air)) {
			return true;
		}
	}

	if (z > 0) {
		if ((blocks[x][z+1][y].type == BlockType::Air)||(blocks[x][z - 1][y].type == BlockType::Air)) {
			return true;
		}
	}
	else {
		if ((blocks[x][z+1][y].type == BlockType::Air)) {
			return true;
		}
	}

	return false;

}


void Chunk::eliminateRayIntersection(glm::vec3 rayOrigin, glm::vec3 rayVector) {

	for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {
		for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
			for (unsigned int y = 0; y < WORLD_HEIGHT; y++) {
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
