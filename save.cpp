#include "save.h"


void worldSaver::closeWorld() {
	file.close();

}

worldSaver::worldSaver(const std::string& inFile, Player *inPlayer) : player(inPlayer),filename(inFile) {


	bool worldExists = std::filesystem::exists(filename) && std::filesystem::file_size(filename) > 0 ;

	//crude method to overwrite world file, delete it if it exists
	if (worldExists&&overwriteFile) {
		std::remove(filename.c_str());
		worldExists = false;
	}

	if(!worldExists) {

		//fstream doesn't like to write a file that doesn't exists when it is in input and output mode, so we do this workaround
		std::ofstream tempWrite(filename, std::ios::binary | std::ios::out);
		
		tempWrite.seekp(0);
		tempWrite.write((char*)&CAM_DEFAULTS::POSITION.x, sizeof(precision));
		tempWrite.write((char*)&CAM_DEFAULTS::POSITION.y, sizeof(precision));
		tempWrite.write((char*)&CAM_DEFAULTS::POSITION.z, sizeof(precision));

		tempWrite.write((char*)&CAM_DEFAULTS::YAW, sizeof(precision));
		tempWrite.write((char*)&CAM_DEFAULTS::PITCH, sizeof(precision));

		uint32_t numChunk = 0;
		tempWrite.write((char*)&numChunk, sizeof(uint32_t));

		tempWrite.close();

	}


	file.open(filename.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary);


	if (worldExists) {

		glm::vec3 tempPos;
		file.read((char*)&tempPos.x, sizeof(precision));
		file.read((char*)&tempPos.y, sizeof(precision));
		file.read((char*)&tempPos.z, sizeof(precision));
		player->setPosition(tempPos);

		file.read((char*)&player->cam.Yaw, sizeof(precision));
		file.read((char*)&player->cam.Pitch, sizeof(precision));

		uint32_t numChunks;
		file.read((char*)&numChunks, sizeof(uint32_t));
		chunkList.resize(numChunks);

		file.read((char*)chunkList.data(), sizeof(int64_t)*numChunks);

		player->cam.updateCameraVectors();

	}

}

void worldSaver::writePosition() {

	file.seekp(0);

	file.write((char*)&player->cam.position.x, sizeof(precision));
	file.write((char*)&player->cam.position.y, sizeof(precision));
	file.write((char*)&player->cam.position.z, sizeof(precision));

	file.write((char*)&player->cam.Yaw, sizeof(precision));
	file.write((char*)&player->cam.Pitch, sizeof(precision));

}

bool worldSaver::tryFillChunk(Chunk* chunk) {
	auto hash = World::genHash(chunk->x, chunk->z);
	auto it = std::find(chunkList.begin(), chunkList.end(), hash);
	auto pos = std::distance(chunkList.begin(), it);
	//we found chunk, fill it and return true
	if (it != chunkList.end()) {
		file.seekg(cameraDataOffset+sizeof(uint32_t) + chunkList.size() * sizeof(int64_t) + chunkDataOffset * pos);
		file.read((char*)chunk->blocks.data(), chunkDataOffset);
		return true;
	}
	//couldn't find chunk in our world file
	else {
		return false;
	}
}


void worldSaver::writeChunk(const Chunk &chunk) {
	
	auto hash = World::genHash(chunk.x, chunk.z);
	auto it = std::find(chunkList.begin(), chunkList.end(), hash);
	auto pos = std::distance(chunkList.begin(), it);
	if (it != chunkList.end()) {
		//we found our chunk, go to it and write the new one
		file.seekp(cameraDataOffset+sizeof(uint32_t) + chunkList.size() * sizeof(int64_t) + chunkDataOffset * pos);
		file.write((char*)chunk.blocks.data(), chunkDataOffset);
		return;
	}
	else {
		//we couldn't find our chunk, add it to the header then append it to the end

		//update chunk list
		//have to do this buffer workaround cause we can't append in the middle of a file
		//make buffer
		auto chunkDataSize = chunkList.size() * CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_HEIGHT;
		std::vector<char> tempBuff(sizeof(int64_t) + chunkDataSize);
		std::memcpy(tempBuff.data(), &hash, sizeof(int64_t));
		file.seekg(cameraDataOffset+4+chunkList.size()*sizeof(int64_t));
		file.read(tempBuff.data() + sizeof(int64_t), chunkDataSize);

		//write new buffer
		file.seekp(cameraDataOffset+4+chunkList.size()*sizeof(int64_t));
		file.write(tempBuff.data(), tempBuff.size());
		chunkList.push_back(hash);

		//update number of chunks
		uint32_t numChunks = chunkList.size();
		file.seekp(cameraDataOffset);
		file.write((char*)&numChunks, sizeof(uint32_t));

		//write chunk itself to end of file
		file.seekp(0,std::ios::end);
		file.write((char*)chunk.blocks.data(), chunkDataOffset);

		return;

	}
}

void ChunkManager::initWorld()
{
	auto r = renderDistance;
	for (int i = r; i >= -r; i--) {
		for (int j = r; j >= -r; j--) {
			loadChunk(player->chunkX+i, player->chunkZ+j);
		}
	}
}

void ChunkManager::checkNewChunk()
{
	int chunkDiffX = player->chunkX-oldChunkX;
	int chunkDiffZ = player->chunkZ-oldChunkZ;

	if (chunkDiffX != 0 || chunkDiffZ != 0) {
		auto r = renderDistance;
		std::set<int64_t> expectedChunks;
		std::set<int64_t> currentChunks;

		for (int i = r; i >= -r; i--) {
			for (int j = r; j >= -r; j--) {
				auto hash = World::genHash(i+player->chunkX, j+player->chunkZ);
				expectedChunks.insert(hash);
			}
		}

		for (const auto& [key, value] : world->chunks) {
			currentChunks.insert(key);
		}

		//load any expected chunks not found
		for (const auto& c : expectedChunks) {
			auto test = currentChunks.end();
			auto exists = std::binary_search(currentChunks.begin(), currentChunks.end(), c);
			//auto it = std::find(currentChunks.begin(), currentChunks.end(), c);
			//if (it == currentChunks.end()) {
			if (!exists) {
				int32_t x,z;
				World::retrieveHash(&x, &z, c);
				loadChunk(x, z);
			}
		}

		//unload old chunks
		for (const auto& c : currentChunks) {
			auto exists = std::binary_search(expectedChunks.begin(), expectedChunks.end(), c);
			if (!exists) {
				unloadChunk(c);
				int32_t x,z;
				World::retrieveHash(&x, &z, c);
			}
		}


	}

	oldChunkX = player->chunkX;
	oldChunkZ = player->chunkZ;
	

}

void ChunkManager::loadChunk(const int& chunkX, const int& chunkZ)
{
	auto hash = World::genHash(chunkX, chunkZ);
	auto foundElement = world->chunks.find(hash);
	//only do something if chunk isn't already loaded
	if (foundElement == world->chunks.end()) {
		Chunk chunk(chunkX,chunkZ);
		bool filledChunk = saver->tryFillChunk(&chunk);
		//if we didn't find chunk in world file, then generate it
		if (!filledChunk) {
			world->populateChunk(chunk);
			saver->writeChunk(chunk);
		}
		world->chunks.insert(std::pair<int64_t,Chunk>(hash,chunk));
		world->renderBlocksDirty = true;
		world->frustumCullDirty = true;
		world->lightDirty = true;
		world->vboDirty = true;
	}

}


void ChunkManager::unloadChunk(const int& chunkX, const int& chunkZ)
{

	auto hash = World::genHash(chunkX, chunkZ);
	world->chunks.erase(hash);
	world->renderBlocksDirty = true;
	world->frustumCullDirty = true;
	world->lightDirty = true;
	world->vboDirty = true;

}

void ChunkManager::unloadChunk(const int64_t& hash)
{
	world->chunks.erase(hash);
	world->renderBlocksDirty = true;
	world->frustumCullDirty = true;
	world->lightDirty = true;
	world->vboDirty = true;
}


