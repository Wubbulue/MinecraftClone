#include "minecraft.h"


void Chunk::empty() {
	//empty block
	Block block;

	for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {
		for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
			for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
				blocks[index(x,z,y)] = block;
			}
		}
	}
}

bool Chunk::isBlockAdjacentToAir(int x, int y, int z) {

	//must make sure we don't check outside chunk bounds

	//check x adjacency
	if ((x > 0)&&(blocks[index(x-1,z,y)].type == BlockType::Air)) {
		return true;
	}
	if ((x < (CHUNK_LENGTH-1))&&(blocks[index(x+1,z,y)].type == BlockType::Air)) {
		return true;
	}

	//check z adjacency
	if ((z > 0)&&(blocks[index(x,z-1,y)].type == BlockType::Air)) {
		return true;
	}
	if ((z < (CHUNK_LENGTH-1))&&(blocks[index(x,z+1,y)].type == BlockType::Air)) {
		return true;
	}

	//check y adjacency
	if ((y > 0)&&(blocks[index(x,z,y-1)].type == BlockType::Air)) {
		return true;
	}
	if ((y < (CHUNK_HEIGHT-1))&&(blocks[index(x,z,y+1)].type == BlockType::Air)) {
		return true;
	}


	return false;

}


void Chunk::eliminateRayIntersection(glm::vec3 rayOrigin, glm::vec3 rayVector) {

	for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {
		for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
			for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
				if (blocks[index(x,z,y)].type != BlockType::Air) {
					Triangle blockTriangles[12];
					getBlockTriangles(x, y, z, blockTriangles);
					for (int i = 0; i < 12; i++) {
						glm::vec3 outPoint;
						if (rayIntersect(rayOrigin, rayVector, blockTriangles + i, outPoint)) {
							blocks[index(x,z,y)].type = BlockType::Air;
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



Block* Chunk::indexAbsolute(BlockPosition pos) {
	pos.x -= x * CHUNK_LENGTH;
	pos.z -= z * CHUNK_LENGTH;
	bool inXRange = (pos.x < CHUNK_LENGTH) && (pos.x >= 0);
	bool inZRange = (pos.z < CHUNK_LENGTH) && (pos.z >= 0);
	bool inYRange = (pos.y < CHUNK_HEIGHT) && (pos.y >= 0);
	assert(inXRange && inYRange && inZRange);

	return blocks + index(pos.x, pos.z, pos.y);
}



Box3 Chunk::getBox() {
	return Box3(glm::vec3(x*CHUNK_LENGTH, 0.0f, z*CHUNK_LENGTH), glm::vec3((x+1)*CHUNK_LENGTH, CHUNK_HEIGHT, (z + 1) * CHUNK_LENGTH));
}


long World::cantorHash(int a, int b)
{
	long A = (unsigned long)(a >= 0 ? 2 * (long)a : -2 * (long)a - 1);
	long B = (unsigned long)(b >= 0 ? 2 * (long)b : -2 * (long)b - 1);
	long C = (long)((A >= B ? A * A + A + B : A + B * B) / 2);
	return a < 0 && b < 0 || a >= 0 && b >= 0 ? C : -C - 1;
}

void World::addChunk(int x, int z) {

	//this is probably copying all of our blocks, probably inneficient TODO
	//chunks.insert(std::pair<long,Chunk>(cantorHash(x,z),chunk));
	Chunk chunk(x,z);

	populateChunk(chunk);

	auto hash = cantorHash(x, z);
	chunks.insert(std::pair<long,Chunk>(hash,chunk));

}



void World::regenerate() {
	seed = std::rand();
	perlin.reseed(seed);
	for (auto& [key, chunk] : chunks)
	{
		chunk.empty();
		populateChunk(chunk);
	}
}


void World::populateChunks() {


	for (auto& [key, chunk] : chunks)
	{
		populateChunk(chunk);
	}

}

void World::populateChunk(Chunk& chunk) {
	int offsetX = chunk.x * CHUNK_LENGTH;
	int offsetZ = chunk.z * CHUNK_LENGTH;
	for (int z = 0; z < CHUNK_LENGTH; ++z)
	{
		for (int x = 0; x < CHUNK_LENGTH; ++x)
		{
			const double noise = perlin.octave2D_01(((x + offsetX) * .01), ((z + offsetZ) * .01), 8);
			uint8_t pixColor = uint8_t(noise * CHUNK_HEIGHT);

			//scuffed solution to not render blocks at height of 1
			//TODO: make this better
			if (pixColor == CHUNK_HEIGHT) pixColor--;

			if (z == 0 || z == CHUNK_LENGTH - 1 || x == 0 || x == CHUNK_LENGTH - 1) {
				chunk.blocks[index(x, z, pixColor)].type = BlockType::Stone;
			}
			else {
				chunk.blocks[index(x, z, pixColor)].type = BlockType::Dirt;
			}

			for (int y = pixColor - 1; y > -1; y--) {
				chunk.blocks[index(x, z, y)].type = BlockType::Stone;
			}

		}

	}
}

void World::mineHoleCast(const Ray& ray, const float& length) {
	float tMin;
	float tMax;
	glm::vec3 ray_start, ray_end;
	ray_start = ray.orig;
	ray_end = ray.dir * length + ray.orig;


	BlockPosition curPos = findBlock(ray_start);
	BlockPosition endPos = findBlock(ray_end);


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


	auto chunk = getChunkContainingBlock(curPos.x, curPos.z);
	while (curPos.x != endPos.x  || curPos.z != endPos.z || curPos.y != endPos.y) {

		//check y bounds TODO: this will prevent picking from rays cast outside y range, implement actual solution of trying a shorter ray
		if ((curPos.y >= CHUNK_HEIGHT)||(curPos.y<0)) {
			return;
		}

		if (!chunk) {
			warn("Chunk loading can't keep up");
			return;
		}

		auto block = chunk->indexAbsolute(curPos);
		if (block->type != BlockType::Air) {
			block->type = BlockType::Air;
		}
		if (tMaxX < tMaxY && tMaxX < tMaxZ) {
			// X-axis traversal.
			curPos.x += stepX;

			//check if we crossed a chunk border and rent a new chunk if we did
			bool crossPositive = (curPos.x % CHUNK_LENGTH == 0) && (stepX > 0);
			bool crossNegative = ((curPos.x+1) % CHUNK_LENGTH == 0) && (stepX < 0);
			if (crossPositive||crossNegative) {
				chunk = getChunkContainingBlock(curPos.x, curPos.z);
			}

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

			bool crossPositive = (curPos.z % CHUNK_LENGTH == 0) && (stepZ > 0);
			bool crossNegative = ((curPos.z+1) % CHUNK_LENGTH == 0) && (stepZ < 0);
			if (crossPositive||crossNegative) {
				chunk = getChunkContainingBlock(curPos.x, curPos.z);
			}

			tMaxZ += tDeltaZ;
		}
	}

}

BlockPosition World::findBlock(glm::vec3 position) {

	//TODO: find better assert solution

	//bool inXRange = position.x >= 0 && position.x <= CHUNK_LENGTH;
	//bool inYRange = position.y >= 0 && position.y <= CHUNK_HEIGHT;
	//bool inZRange = position.z >= 0 && position.z <= CHUNK_LENGTH;

	//assert(inXRange && inYRange && inZRange);
	//if (!inXRange || !inYRange || !inZRange) {
	//	printf("Not in range\n");
	//}

	int intx = std::floor(position.x);
	int inty = std::floor(position.y);
	int intz = std::floor(position.z);


	BlockPosition block_position = {
		intx,inty,intz
	};

	return block_position;


}

Chunk* World::getChunkContainingBlock(const int& x,const int& z) {
	
	int chunkX = x / CHUNK_LENGTH;
	int chunkZ = z / CHUNK_LENGTH;

	if (x < 0) {
		chunkX -= 1;
	}

	if (z < 0) {
		chunkZ -= 1;
	}

	auto hash = cantorHash(chunkX, chunkZ);
	auto elementFound = chunks.find(hash);
	if (elementFound != chunks.end()) {
		return &(elementFound->second);
	}
	else {
		//chunks isn't generated!!!
		warn("Chunk loading is behind");
		return nullptr;
	}


}

bool World::findFirstSolid(const Ray& ray, const float& length, BlockPosition& pos) {

	float tMin;
	float tMax;
	glm::vec3 ray_start, ray_end;
	ray_start = ray.orig;
	ray_end = ray.dir * length + ray.orig;


	BlockPosition curPos = findBlock(ray_start);
	BlockPosition endPos = findBlock(ray_end);

	//auto chunkTemp = getChunkContainingBlock(endPos.x, endPos.z);
	//BlockType type;
	//if (chunkTemp) {
	//	auto blockTemp = chunkTemp->indexAbsolute(endPos);
	//	type = blockTemp->type;
	//	blockTemp->type = BlockType::Stone;
	//}


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



	auto chunk = getChunkContainingBlock(curPos.x, curPos.z);
	while (curPos.x != endPos.x  || curPos.z != endPos.z || curPos.y != endPos.y) {

		//check y bounds TODO: this will prevent picking from rays cast outside y range, implement actual solution of trying a shorter ray
		if ((curPos.y >= CHUNK_HEIGHT)||(curPos.y<0)) {
			return false;
		}

		if (!chunk) {
			warn("Chunk loading can't keep up");
			return false;
		}

		auto block = chunk->indexAbsolute(curPos);
		if (block->type != BlockType::Air) {
			pos = curPos;
			return true;
		}

		if (tMaxX < tMaxY && tMaxX < tMaxZ) {
			// X-axis traversal.
			curPos.x += stepX;

			//check if we crossed a chunk border and rent a new chunk if we did
			bool crossPositive = (curPos.x % CHUNK_LENGTH == 0) && (stepX > 0);
			bool crossNegative = ((curPos.x+1) % CHUNK_LENGTH == 0) && (stepX < 0);
			if (crossPositive||crossNegative) {
				chunk = getChunkContainingBlock(curPos.x, curPos.z);
			}

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

			bool crossPositive = (curPos.z % CHUNK_LENGTH == 0) && (stepZ > 0);
			bool crossNegative = ((curPos.z+1) % CHUNK_LENGTH == 0) && (stepZ < 0);
			if (crossPositive||crossNegative) {
				chunk = getChunkContainingBlock(curPos.x, curPos.z);
			}

			tMaxZ += tDeltaZ;
		}
	}
	return false;

}

uint32_t World::numBlocks() {

	uint32_t numBlocks = 0;

	for (const auto& [key, chunk] : chunks)
	{
		for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {

			for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
				for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {

					auto blockType = chunk.blocks[index(x, z, y)].type;
					if (blockType == BlockType::Air) {
						continue;
					}
					numBlocks++;

				}
			}
		}

	}
	return numBlocks;
}
