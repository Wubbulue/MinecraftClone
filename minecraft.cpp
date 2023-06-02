#include "minecraft.h"
#include <random>


void Chunk::empty() {
	//empty block
	Block block;

	for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {
		for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
			for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
				blocks[index(x, z, y)] = block;
			}
		}
	}
}

bool Chunk::isBlockAdjacentToAir(int x, int y, int z) {

	//must make sure we don't check outside chunk bounds

	//check x adjacency
	if ((x > 0) && (blocks[index(x - 1, z, y)].type == BlockTypes::Air)) {
		return true;
	}
	if ((x < (CHUNK_LENGTH - 1)) && (blocks[index(x + 1, z, y)].type == BlockTypes::Air)) {
		return true;
	}

	//check z adjacency
	if ((z > 0) && (blocks[index(x, z - 1, y)].type == BlockTypes::Air)) {
		return true;
	}
	if ((z < (CHUNK_LENGTH - 1)) && (blocks[index(x, z + 1, y)].type == BlockTypes::Air)) {
		return true;
	}

	//check y adjacency
	if ((y > 0) && (blocks[index(x, z, y - 1)].type == BlockTypes::Air)) {
		return true;
	}
	if ((y < (CHUNK_HEIGHT - 1)) && (blocks[index(x, z, y + 1)].type == BlockTypes::Air)) {
		return true;
	}


	return false;

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


std::string BlockTypes::blockTypeToString(BlockType type) {
	return blockTypeStrings[(size_t)type];
}



Block* Chunk::indexAbsolute(BlockPosition pos) {
	pos.x -= x * CHUNK_LENGTH;
	pos.z -= z * CHUNK_LENGTH;
	bool inXRange = (pos.x < CHUNK_LENGTH) && (pos.x >= 0);
	bool inZRange = (pos.z < CHUNK_LENGTH) && (pos.z >= 0);
	bool inYRange = (pos.y < CHUNK_HEIGHT) && (pos.y >= 0);
	assert(inXRange && inYRange && inZRange);

	return blocks.data() + index(pos.x, pos.z, pos.y);
}



Box3 Chunk::getBox() {
	return Box3(glm::vec3(x * CHUNK_LENGTH, 0.0f, z * CHUNK_LENGTH), glm::vec3((x + 1) * CHUNK_LENGTH, CHUNK_HEIGHT, (z + 1) * CHUNK_LENGTH));
}


int64_t World::genHash(int32_t a, int32_t b)
{
	//UNDERSTAND: i don't know why we have to cast like this here
	return (uint64_t)a << 32 | (uint32_t)b;
}

void World::retrieveHash(int32_t* a, int32_t* b, int64_t c)
{
	*a = c >> 32;
	*b = c & 0xFFFFFFFF;
}

void World::addChunk(int x, int z) {


	auto hash = genHash(x, z);
	if (chunks.find(hash) != chunks.end()) {
		std::cout << "Chunk already exists, nothing added" << std::endl;
		return;
	}

	//this is probably copying all of our blocks, probably inneficient TODO
	Chunk chunk(x, z);

	populateChunk(chunk);

	chunks.insert(std::pair<int64_t, Chunk>(hash, chunk));

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

			chunk.blocks[index(x, z, pixColor)].type = BlockTypes::Dirt;

			for (int y = pixColor - 1; y > -1; y--) {
				chunk.blocks[index(x, z, y)].type = BlockTypes::Stone;
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
		tMaxX = ((curPos.x + 1) - ray_start.x) / ray.dir.x;
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
		tMaxY = ((curPos.y + 1) - ray_start.y) / ray.dir.y;
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
		tMaxZ = ((curPos.z + 1) - ray_start.z) / ray.dir.z;
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
	while (curPos.x != endPos.x || curPos.z != endPos.z || curPos.y != endPos.y) {

		//check y bounds TODO: this will prevent picking from rays cast outside y range, implement actual solution of trying a shorter ray
		if ((curPos.y >= CHUNK_HEIGHT) || (curPos.y < 0)) {
			return;
		}

		if (!chunk) {
			return;
		}

		auto block = chunk->indexAbsolute(curPos);
		if (block->type != BlockTypes::Air) {
			block->type = BlockTypes::Air;
			renderBlocksDirty = true;
		}
		if (tMaxX < tMaxY && tMaxX < tMaxZ) {
			// X-axis traversal.
			curPos.x += stepX;

			//check if we crossed a chunk border and rent a new chunk if we did
			bool crossPositive = (curPos.x % CHUNK_LENGTH == 0) && (stepX > 0);
			bool crossNegative = ((curPos.x + 1) % CHUNK_LENGTH == 0) && (stepX < 0);
			if (crossPositive || crossNegative) {
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
			bool crossNegative = ((curPos.z + 1) % CHUNK_LENGTH == 0) && (stepZ < 0);
			if (crossPositive || crossNegative) {
				chunk = getChunkContainingBlock(curPos.x, curPos.z);
			}

			tMaxZ += tDeltaZ;
		}
	}

}

BlockPosition World::findBlock(const glm::vec3& position) {

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

void World::findChunk(const glm::vec3& position, int* chunkX, int* chunkZ)
{

	int intx = std::floor(position.x);
	int intz = std::floor(position.z);

	*chunkX = intx / CHUNK_LENGTH;
	*chunkZ = intz / CHUNK_LENGTH;

	if (intx < 0) {
		*chunkX -= 1;
	}

	if (intz < 0) {
		*chunkZ -= 1;
	}

}

Chunk* World::getChunkContainingBlock(const int& x, const int& z) {

	int chunkX = x / CHUNK_LENGTH;
	int chunkZ = z / CHUNK_LENGTH;

	if (x < 0 && (x % 16 != 0)) {
		chunkX -= 1;
	}

	if (z < 0 && (z % 16 != 0)) {
		chunkZ -= 1;
	}

	return getChunk(chunkX, chunkZ);
}

Chunk* World::getChunkContainingBlock(const BlockPosition& pos) {

	int chunkX = pos.x / CHUNK_LENGTH;
	int chunkZ = pos.z / CHUNK_LENGTH;

	if (pos.x < 0 && (pos.x % 16 != 0)) {
		chunkX -= 1;
	}

	if (pos.z < 0 && (pos.z % 16 != 0)) {
		chunkZ -= 1;
	}

	return getChunk(chunkX, chunkZ);
}

Chunk* World::getChunkContainingPosition(const glm::vec3& position)
{
	auto blockPos = findBlock(position);
	return getChunkContainingBlock(blockPos.x, blockPos.z);
}

Block* World::getBlock(const BlockPosition& pos)
{
	auto chunk = getChunkContainingBlock(pos);

	if (!chunk) {
		return nullptr;
	}
	else {
		auto block = chunk->indexAbsolute(pos);
		return block;
	}
}

Chunk* World::getChunk(const int& x, const int& z)
{
	auto hash = genHash(x, z);
	auto elementFound = chunks.find(hash);
	if (elementFound != chunks.end()) {
		return &(elementFound->second);
	}
	else {
		return nullptr;
	}
}

//TODO: this is unusuably slow
bool World::isBlockAdjacentToAir(BlockPosition pos)
{

	//must make sure we don't check outside chunk bounds

	Block* block;

	//check x adjacency
	pos.x++;
	block = getBlock(pos);
	if ((block) && (block->type == BlockTypes::Air)) {
		return true;
	}
	pos.x--;

	pos.x--;
	block = getBlock(pos);
	if ((block) && (block->type == BlockTypes::Air)) {
		return true;
	}
	pos.x++;

	//check z adjacency
	pos.z++;
	block = getBlock(pos);
	if ((block) && (block->type == BlockTypes::Air)) {
		return true;
	}
	pos.z--;

	pos.z--;
	block = getBlock(pos);
	if ((block) && (block->type == BlockTypes::Air)) {
		return true;
	}
	pos.z++;

	if (pos.y < (CHUNK_HEIGHT - 1)) {
		pos.y++;
		block = getBlock(pos);
		if ((block) && (block->type == BlockTypes::Air)) {
			return true;
		}
		pos.y--;
	}

	if (pos.y > 0) {
		pos.y--;
		block = getBlock(pos);
		if ((block) && (block->type == BlockTypes::Air)) {
			return true;
		}
		pos.y++;
	}

	return false;
}

int World::customIndex(int x, int z, int y)
{
	return (x)+((z)*worldLength) + ((y)*worldLength * worldLength);
}

int World::customIndex(const BlockPosition& pos) {
	return (pos.x) + ((pos.z) * worldLength) + ((pos.y) * worldLength * worldLength);
}



BlockPosition absoluteToRenderSpace(BlockPosition& pos) {

	return { 1,1,1 };
}

BlockPosition renderSpaceToAbsoluteSpace(BlockPosition& pos) {

	return { 1,1,1 };
}

void World::propogateLight(BlockPosition& pos) {


	//TODO: out of bounds checking
	auto startingLightLevel = lightLevel[customIndex(pos)];
	std::queue<BlockPosition> toVisit;

	//this being full size might be a waste?
	std::unordered_set<BlockPosition> alreadyVisited;
	alreadyVisited.insert(pos);
	BlockPosition tempPos;

	uint8_t* lightPtr;
	uint8_t newLightLevel = startingLightLevel - 1;

	tempPos = {pos.x,pos.y,pos.z-1};
	lightPtr = lightLevel.data() + customIndex(tempPos);
	if (*lightPtr < newLightLevel) {
		toVisit.push(tempPos);
		*lightPtr = newLightLevel;
	}

	tempPos = {pos.x,pos.y,pos.z+1};
	lightPtr = lightLevel.data() + customIndex(tempPos);
	if (*lightPtr < newLightLevel) {
		toVisit.push(tempPos);
		*lightPtr = newLightLevel;
	}

	tempPos = {pos.x,pos.y-1,pos.z};
	lightPtr = lightLevel.data() + customIndex(tempPos);
	if (*lightPtr < newLightLevel) {
		toVisit.push(tempPos);
		*lightPtr = newLightLevel;
	}

	tempPos = {pos.x,pos.y+1,pos.z};
	lightPtr = lightLevel.data() + customIndex(tempPos);
	if (*lightPtr < newLightLevel) {
		toVisit.push(tempPos);
		*lightPtr = newLightLevel;
	}

	tempPos = {pos.x-1,pos.y,pos.z};
	lightPtr = lightLevel.data() + customIndex(tempPos);
	if (*lightPtr < newLightLevel) {
		toVisit.push(tempPos);
		*lightPtr = newLightLevel;
	}

	tempPos = {pos.x+1,pos.y,pos.z};
	lightPtr = lightLevel.data() + customIndex(tempPos);
	if (*lightPtr < newLightLevel) {
		toVisit.push(tempPos);
		*lightPtr = newLightLevel;
	}





	while (!toVisit.empty()) {
		BlockPosition blockToVisit = toVisit.front();
		toVisit.pop();

		if (alreadyVisited.count(blockToVisit)) {
			//If we have already visited this, then don't keep looking
			continue;
		}

		startingLightLevel = lightLevel[customIndex(blockToVisit)];
		if (startingLightLevel <= 1) {
			//we can't spread a light level of 0
			continue;
		}

		alreadyVisited.insert(blockToVisit);

		newLightLevel = startingLightLevel - 1;

		//Otherwise, spread the light!
		tempPos = {blockToVisit.x,blockToVisit.y,blockToVisit.z-1};
		lightPtr = lightLevel.data() + customIndex(tempPos);
		if (*lightPtr < newLightLevel) {
			toVisit.push(tempPos);
			*lightPtr = newLightLevel;
		}

		tempPos = {blockToVisit.x,blockToVisit.y,blockToVisit.z+1};
		lightPtr = lightLevel.data() + customIndex(tempPos);
		if (*lightPtr < newLightLevel) {
			toVisit.push(tempPos);
			*lightPtr = newLightLevel;
		}

		tempPos = {blockToVisit.x,blockToVisit.y-1,blockToVisit.z};
		lightPtr = lightLevel.data() + customIndex(tempPos);
		if (*lightPtr < newLightLevel) {
			toVisit.push(tempPos);
			*lightPtr = newLightLevel;
		}

		tempPos = {blockToVisit.x,blockToVisit.y+1,blockToVisit.z};
		lightPtr = lightLevel.data() + customIndex(tempPos);
		if (*lightPtr < newLightLevel) {
			toVisit.push(tempPos);
			*lightPtr = newLightLevel;
		}

		tempPos = {blockToVisit.x-1,blockToVisit.y,blockToVisit.z};
		lightPtr = lightLevel.data() + customIndex(tempPos);
		if (*lightPtr < newLightLevel) {
			toVisit.push(tempPos);
			*lightPtr = newLightLevel;
		}

		tempPos = {blockToVisit.x+1,blockToVisit.y,blockToVisit.z};
		lightPtr = lightLevel.data() + customIndex(tempPos);
		if (*lightPtr < newLightLevel) {
			toVisit.push(tempPos);
			*lightPtr = newLightLevel;
		}

	}
	
	




}

void World::getBlocksToRenderThreaded(int chunkX, int chunkZ, const Frustum& camFrustum)
{

	//cull obfuscated
	if (renderBlocksDirty)
	{

		renderBlocksDirty = false;

		//gather all blocks to operate on
		{
			int chunkNum = 0, numChunkX = 0, numChunkZ = 0;

			for (int i = chunkX - renderDistance; i < chunkX + renderDistance + 1; i++) {

				numChunkZ = 0;
				for (int j = chunkZ - renderDistance; j < chunkZ + renderDistance + 1; j++) {
					auto chunk = getChunk(i, j);
					assert(chunk);

					for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {

						for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
							for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
								int largeIndex = customIndex(x + (CHUNK_LENGTH * numChunkX), z + (CHUNK_LENGTH * numChunkZ), y);
								int index = index(x, z, y);
								fullWorld[largeIndex] = chunk->blocks[index];

							}
						}
					}
					chunkNum++;

					numChunkZ++;
				}

				numChunkX++;

			}
		}

		airCulled = fullWorld;

		//cull blocks that aren't adjacent to air
		{
			int originX = chunkX * CHUNK_LENGTH;
			int originZ = chunkZ * CHUNK_LENGTH;

			int minX = originX - (CHUNK_LENGTH * renderDistance);
			int minZ = originZ - (CHUNK_LENGTH * renderDistance);

			int maxX = originX + (CHUNK_LENGTH * renderDistance) - 1;
			int maxZ = originZ + (CHUNK_LENGTH * renderDistance) - 1;

			int chunkNum = 0;
			int numChunkX = 0;
			int numChunkZ = 0;


			std::vector<std::future<void>> futures;
			ThreadPool& pool = ThreadPool::shared_instance();



			for (int i = chunkX - renderDistance; i < chunkX + renderDistance + 1; i++) {

				numChunkZ = 0;
				for (int j = chunkZ - renderDistance; j < chunkZ + renderDistance + 1; j++) {

					futures.emplace_back(pool.enqueue([chunkNum, this, numChunkX, numChunkZ, minX, minZ, maxX, maxZ, i, j] {
						for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {
							for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
								for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {

									int offset = chunkNum * chunkDataOffset;
									int blockX = x + numChunkX * CHUNK_LENGTH;
									int blockZ = z + numChunkZ * CHUNK_LENGTH;
									int index = customIndex(blockX, blockZ, y);

									int absX = x + (i * CHUNK_LENGTH);
									int absZ = z + (j * CHUNK_LENGTH);

									if (fullWorld[index].type == BlockTypes::Air) {
										//blocks[posInArray].type = BlockTypes::Air; // probably don't need this line
										continue;
									}



									if ((absX > minX) && (fullWorld[customIndex(blockX - 1, blockZ, y)].type == BlockTypes::Air)) {
										continue;
									}
									if ((absX < maxX) && (fullWorld[customIndex(blockX + 1, blockZ, y)].type == BlockTypes::Air)) {
										continue;
									}

									//check z adjacency
									if ((absZ > minZ) && (fullWorld[customIndex(blockX, blockZ - 1, y)].type == BlockTypes::Air)) {
										continue;
									}
									if ((absZ < maxZ) && (fullWorld[customIndex(blockX, blockZ + 1, y)].type == BlockTypes::Air)) {
										continue;
									}

									//check y adjacency
									if ((y > 0) && (fullWorld[customIndex(blockX, blockZ, y - 1)].type == BlockTypes::Air)) {
										continue;
									}

									if ((y < (CHUNK_HEIGHT - 1)) && (fullWorld[customIndex(blockX, blockZ, y + 1)].type == BlockTypes::Air)) {
										continue;
									}

									airCulled[index].type = BlockTypes::Air;

								}
							}
						}
						}));


					chunkNum++;

					numChunkZ++;
				}

				numChunkX++;
			}

			for (const auto& f : futures) {
				f.wait();
			}
		}
	}


	std::atomic_int numBlocksToRender = 0;

	//frustum cull from camera
	{
		fullCulled = airCulled;
		ThreadPool& pool = ThreadPool::shared_instance();
		std::vector<std::future<void>> futures;

		int chunkNum = 0, numChunkX = 0, numChunkZ = 0;

		for (int i = chunkX - renderDistance; i < chunkX + renderDistance + 1; i++) {

			numChunkZ = 0;
			for (int j = chunkZ - renderDistance; j < chunkZ + renderDistance + 1; j++) {


				float offsetX = i * CHUNK_LENGTH;
				float offsetZ = j * CHUNK_LENGTH;


				futures.emplace_back(pool.enqueue([chunkNum, numChunkX, numChunkZ, offsetX, offsetZ, camFrustum, i, j, this, &numBlocksToRender] {
					for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {

						for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
							for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {


								auto block = fullCulled.data() + customIndex(x + (numChunkX * CHUNK_LENGTH), z + (numChunkZ * CHUNK_LENGTH), y);
								if (block->type == BlockTypes::Air) {
									continue;
								}


								glm::vec3 center(float(x + 0.5f) + offsetX, float(y + 0.5f), float(z + 0.5f) + offsetZ);

								SquareAABB square(center, 0.5f);
								bool isOnFurstum = square.isOnFrustum(camFrustum);

								if (!isOnFurstum) {
									block->type = BlockTypes::Air;
									continue;
								}

								numBlocksToRender++;

							}
						}
					}
					}));

				chunkNum++;
				numChunkZ++;

			}

			numChunkX++;

		}

		for (const auto& f : futures) {
			f.wait();
		}
	}

	//progate light
	{
		
		std::fill(lightLevel.begin(), lightLevel.end(), 3);
		int chunkNum = 0, numChunkX = 0, numChunkZ = 0;

		for (int i = chunkX - renderDistance; i < chunkX + renderDistance + 1; i++) {

			numChunkZ = 0;
			for (int j = chunkZ - renderDistance; j < chunkZ + renderDistance + 1; j++) {


				int offsetX = i * CHUNK_LENGTH;
				int offsetZ = j * CHUNK_LENGTH;


				{

					for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {

						for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
							for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
								BlockPosition relativePos = { x + (numChunkX * CHUNK_LENGTH), y,  z + (numChunkZ * CHUNK_LENGTH)};
								auto ind = customIndex(relativePos);
								auto block = fullWorld.data() + ind;
								if (block->type == BlockTypes::Planck) {
									lightLevel[ind] = 15;
									propogateLight(relativePos);

								}
							}
						}
					}


					//}));
				}

				chunkNum++;
				numChunkZ++;

			}

			numChunkX++;

		}

		//pack data into our vbo
		{

			ThreadPool& pool = ThreadPool::shared_instance();
			std::vector<std::future<void>> futures;

			glBindVertexArray(VAO);

			// numblocks * (5 floats + 2 char) * 36 verts
			auto bufSize = numBlocksToRender * dataPerVert * numVertsPerBlock;
			char* buffer = new char[bufSize];
			int chunkNum = 0, numChunkX = 0, numChunkZ = 0;

			int bufferWritePos = 0;

			for (int i = chunkX - renderDistance; i < chunkX + renderDistance + 1; i++) {

				numChunkZ = 0;
				for (int j = chunkZ - renderDistance; j < chunkZ + renderDistance + 1; j++) {


					int offsetX = i * CHUNK_LENGTH;
					int offsetZ = j * CHUNK_LENGTH;


					{


						for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {

							for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
								for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {

									auto ind = customIndex(x + (numChunkX * CHUNK_LENGTH), z + (numChunkZ * CHUNK_LENGTH), y);
									auto block = fullCulled.data() + ind;

									if (block->type == BlockTypes::Air) {
										continue;
									}

									const uint8_t* facesPointer = dirtFaces;

									if (block->type == BlockTypes::Dirt) {
										facesPointer = dirtFaces;
									}
									else if (block->type == BlockTypes::Stone) {
										facesPointer = stoneFaces;
									}
									else if (block->type == BlockTypes::Planck) {
										facesPointer = planckFaces;
									}

									glm::vec3 center(float(x + 0.5f) + offsetX, float(y + 0.5f), float(z + 0.5f) + offsetZ);

									//uint8_t randLightLevel = ((x + offsetX) * (z + offsetZ)) % 16;
									// uint8_t randLightLevel = 4;
									uint8_t randLightLevel = lightLevel[ind];
									for (int a = 0; a < 36; a++) {
										{
											int index = (a * 3);
											float current = vertPositions[index] + center.x;
											memcpy(buffer + bufferWritePos, &current, sizeof(float));
											bufferWritePos += sizeof(float);
											index++;

											current = vertPositions[index] + center.y;
											memcpy(buffer + bufferWritePos, &current, sizeof(float));
											bufferWritePos += sizeof(float);
											index++;

											current = vertPositions[index] + center.z;
											memcpy(buffer + bufferWritePos, &current, sizeof(float));
											bufferWritePos += sizeof(float);

										}

										for (int b = 0; b < 2; b++) {
											int index = b + (a * 2);
											memcpy(buffer + bufferWritePos, texPositions + index, sizeof(float));
											bufferWritePos += sizeof(float);
										}
										memcpy(buffer + bufferWritePos, facesPointer + a, sizeof(uint8_t));
										bufferWritePos += sizeof(uint8_t);
										memcpy(buffer + bufferWritePos, &randLightLevel, sizeof(uint8_t));
										bufferWritePos += sizeof(uint8_t);
									}

								}
							}
						}


						//}));
					}

					chunkNum++;
					numChunkZ++;

				}

				numChunkX++;

			}


			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, bufSize, buffer);
			glDrawArrays(GL_TRIANGLES, 0, numVertsPerBlock * numBlocksToRender);

			delete[] buffer;
		}
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
	//	blockTemp->type = BlockTypes::Stone;
	//}


	int stepX;
	float tDeltaX;
	float tMaxX;
	if (ray.dir.x > 0.0) {
		stepX = 1;
		tDeltaX = 1.0f / ray.dir.x;
		tMaxX = ((curPos.x + 1) - ray_start.x) / ray.dir.x;
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
		tMaxY = ((curPos.y + 1) - ray_start.y) / ray.dir.y;
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
		tMaxZ = ((curPos.z + 1) - ray_start.z) / ray.dir.z;
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
	while (curPos.x != endPos.x || curPos.z != endPos.z || curPos.y != endPos.y) {

		//check y bounds TODO: this will prevent picking from rays cast outside y range, implement actual solution of trying a shorter ray
		if ((curPos.y >= CHUNK_HEIGHT) || (curPos.y < 0)) {
			return false;
		}

		if (!chunk) {
			return false;
		}

		auto block = chunk->indexAbsolute(curPos);
		if (block->type != BlockTypes::Air) {
			pos = curPos;
			return true;
		}

		if (tMaxX < tMaxY && tMaxX < tMaxZ) {
			// X-axis traversal.
			curPos.x += stepX;

			//check if we crossed a chunk border and rent a new chunk if we did
			bool crossPositive = (curPos.x % CHUNK_LENGTH == 0) && (stepX > 0);
			bool crossNegative = ((curPos.x + 1) % CHUNK_LENGTH == 0) && (stepX < 0);
			if (crossPositive || crossNegative) {
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
			bool crossNegative = ((curPos.z + 1) % CHUNK_LENGTH == 0) && (stepZ < 0);
			if (crossPositive || crossNegative) {
				chunk = getChunkContainingBlock(curPos.x, curPos.z);
			}

			tMaxZ += tDeltaZ;
		}
	}
	return false;

}

bool World::getPlaceBlock(const Ray& ray, const float& length, BlockPosition& pos) {


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
	//	blockTemp->type = BlockTypes::Stone;
	//}


	int stepX;
	float tDeltaX;
	float tMaxX;
	if (ray.dir.x > 0.0) {
		stepX = 1;
		tDeltaX = 1.0f / ray.dir.x;
		tMaxX = ((curPos.x + 1) - ray_start.x) / ray.dir.x;
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
		tMaxY = ((curPos.y + 1) - ray_start.y) / ray.dir.y;
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
		tMaxZ = ((curPos.z + 1) - ray_start.z) / ray.dir.z;
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



	BlockPosition prevPos = curPos;
	auto chunk = getChunkContainingBlock(curPos.x, curPos.z);
	while (curPos.x != endPos.x || curPos.z != endPos.z || curPos.y != endPos.y) {

		//check y bounds TODO: this will prevent picking from rays cast outside y range, implement actual solution of trying a shorter ray
		if ((curPos.y >= CHUNK_HEIGHT) || (curPos.y < 0)) {
			return false;
		}

		if (!chunk) {
			return false;
		}

		auto block = chunk->indexAbsolute(curPos);
		if (block->type != BlockTypes::Air) {
			if (prevPos != curPos) {
				pos = prevPos;
				return true;
			}
			else {
				return false;
			}
		}

		prevPos = curPos;

		if (tMaxX < tMaxY && tMaxX < tMaxZ) {
			// X-axis traversal.
			curPos.x += stepX;

			//check if we crossed a chunk border and rent a new chunk if we did
			bool crossPositive = (curPos.x % CHUNK_LENGTH == 0) && (stepX > 0);
			bool crossNegative = ((curPos.x + 1) % CHUNK_LENGTH == 0) && (stepX < 0);
			if (crossPositive || crossNegative) {
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
			bool crossNegative = ((curPos.z + 1) % CHUNK_LENGTH == 0) && (stepZ < 0);
			if (crossPositive || crossNegative) {
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
					if (blockType == BlockTypes::Air) {
						continue;
					}
					numBlocks++;

				}
			}
		}

	}
	return numBlocks;
}

void World::removeBlock(const BlockPosition& pos) {
	auto chunk = getChunkContainingBlock(pos);
	if (chunk) {
		renderBlocksDirty = true;
		auto block = chunk->indexAbsolute(pos);
		block->type = BlockTypes::Air;
	}
}

void World::initOpenGL() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//this might be waaay to much gpu memory to reserve, we will see...
	const int maxNumBlockToRender = 20000;
	//not sure if this is good practice to give it nullptr, just want to reserve the memory rn, will set later
	glBufferData(GL_ARRAY_BUFFER, maxNumBlockToRender * totalDataPerBlock, nullptr, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, dataPerVert, (void*)0);
	glEnableVertexAttribArray(0);

	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, dataPerVert, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// face type attribute
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, dataPerVert, (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, dataPerVert, (void*)(5 * sizeof(float) + 1));
	glEnableVertexAttribArray(3);
}
