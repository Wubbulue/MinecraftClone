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

BlockPosition Chunk::findBlock(glm::vec3 position) {

	//TODO: find better assert solution

	bool inXRange = position.x >= 0 && position.x <= CHUNK_LENGTH;
	bool inYRange = position.y >= 0 && position.y <= CHUNK_HEIGHT;
	bool inZRange = position.z >= 0 && position.z <= CHUNK_LENGTH;

	assert(inXRange && inYRange && inZRange);
	//if (!inXRange || !inYRange || !inZRange) {
	//	printf("Not in range\n");
	//}

	uint16_t intx = std::floor(position.x);
	uint16_t inty = std::floor(position.y);
	uint16_t intz = std::floor(position.z);

	//include positions at chunk border
	if (intx == CHUNK_LENGTH) intx--;
	if (inty == CHUNK_HEIGHT) inty--;
	if (intz == CHUNK_LENGTH) intz--;

	BlockPosition block_position = {
		intx,inty,intz
	};

	return block_position;


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


void Chunk::traverseUntilSolid(const Ray& ray) {
	float tMin;
	float tMax;
	Box3 box = getBox();
	glm::vec3 ray_start, ray_end;
	if (!box.checkIfInside(ray.orig)) {
		//if it doesn't intersect, then return
		if (!box.intersect(ray, ray_start, ray_end)) {
			return;
		}
	}
	else {
		//only need ending ray intersect, beginning will be our position
		box.intersect(ray, glm::vec3(), ray_end);
		ray_start = ray.orig;
	}

	BlockPosition curPos = findBlock(ray_start - box.bounds[0]);
	const BlockPosition endPos = findBlock(ray_end - box.bounds[0]);

	int stepX;
	float tDeltaX;
	float tMaxX;
	if (ray.dir.x > 0.0) {
		stepX = 1;
		tDeltaX = 1.0f / ray.dir.x;
		tMaxX = ((curPos.x+1) - ray_start.x) / ray.dir.x;
	}
	else if (ray.dir.x < 0.0) {
		stepX = -1;
		tDeltaX = 1.0f / -ray.dir.x;
		tMaxX = (curPos.x - ray_start.x) / ray.dir.x;
	}
	else {
		//never increment x
		stepX = 0;
		tMaxX = std::numeric_limits<float>::max();
		tDeltaX = std::numeric_limits<float>::max();
	}

	int stepY;
	float tDeltaY;
	float tMaxY;
	if (ray.dir.y > 0.0) {
		stepY = 1;
		tDeltaY = 1.0f / ray.dir.y;
		tMaxY = ((curPos.y+1) - ray_start.y) / ray.dir.y;
	}
	else if (ray.dir.y < 0.0) {
		stepY = -1;
		tDeltaY = 1.0f / -ray.dir.y;
		tMaxY = (curPos.y - ray_start.y) / ray.dir.y;
	}
	else {
		stepY = 0;
		tMaxY = std::numeric_limits<float>::max();
		tDeltaY = std::numeric_limits<float>::max();
	}

	int stepZ;
	float tDeltaZ;
	float tMaxZ;
	if (ray.dir.z > 0.0) {
		stepZ = 1;
		tDeltaZ = 1.0f / ray.dir.z;
		tMaxZ = ((curPos.z+1) - ray_start.z) / ray.dir.z;
	}
	else if (ray.dir.z < 0.0) {
		stepZ = -1;
		tDeltaZ = 1.0f / -ray.dir.z;
		tMaxZ = (curPos.z - ray_start.z) / ray.dir.z;
	}
	else {
		stepZ = 0;
		tMaxZ = std::numeric_limits<float>::max();
		tDeltaZ = std::numeric_limits<float>::max();
	}

	while (curPos.x != endPos.x  || curPos.z != endPos.z || curPos.y != endPos.y) {
		if (blocks[curPos.x][curPos.z][curPos.y].type != BlockType::Air) {
			blocks[curPos.x][curPos.z][curPos.y].type = BlockType::Air;
			//return;
		}
		if (tMaxX < tMaxY && tMaxX < tMaxZ) {
			// X-axis traversal.
			curPos.x += stepX;
			tMaxX += tDeltaX;
		}
		else if (tMaxY < tMaxZ) {
			// Y-axis traversal.
			curPos.y += stepY;
			tMaxY += tDeltaY;
		}
		else {
			// Z-axis traversal.
			curPos.z += stepZ;
			tMaxZ += tDeltaZ;
		}
	}
}


Box3 Chunk::getBox() {
	return Box3(glm::vec3(x*CHUNK_LENGTH, 0.0f, z*CHUNK_LENGTH), glm::vec3((x+1)*CHUNK_LENGTH, CHUNK_HEIGHT, (z + 1) * CHUNK_LENGTH));
}


