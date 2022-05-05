#include "save.h"

bool loadWorld(const std::string& filename, World& world, Camera& cam) {
	std::ifstream input(filename.c_str(), std::ios::in | std::ios::binary);

	if (!input.is_open()) {
		std::cout << "Error with opening" << filename << ". World not loaded." << std::endl;
		return false;
	}


}


void worldSaver::closeWorld() {
	file.close();

}

worldSaver::worldSaver(const std::string& inFile, Camera *inCam) : cam(inCam),filename(inFile) {


	bool worldExists = std::filesystem::exists(filename) && std::filesystem::file_size(filename) > 0 ;

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

		file.read((char*)&cam->position.x, sizeof(precision));
		file.read((char*)&cam->position.y, sizeof(precision));
		file.read((char*)&cam->position.z, sizeof(precision));

		file.read((char*)&cam->Yaw, sizeof(precision));
		file.read((char*)&cam->Pitch, sizeof(precision));

		uint32_t numChunks;
		file.read((char*)&numChunks, sizeof(uint32_t));
		chunkList.resize(numChunks);
		for (int i = 0; i < numChunks; i++) {
			file.read((char*)&chunkList[i], sizeof(long));
		}

		cam->updateCameraVectors();

	}

}

void worldSaver::writePosition() {

	file.seekp(0);

	file.write((char*)&cam->position.x, sizeof(precision));
	file.write((char*)&cam->position.y, sizeof(precision));
	file.write((char*)&cam->position.z, sizeof(precision));

	file.write((char*)&cam->Yaw, sizeof(precision));
	file.write((char*)&cam->Pitch, sizeof(precision));

}

bool worldSaver::tryFillChunk(Chunk* chunk) {
	long hash = World::cantorHash(chunk->x, chunk->z);
	auto it = std::find(chunkList.begin(), chunkList.end(), hash);
	auto pos = std::distance(chunkList.begin(), it);
	//we found chunk, fill it and return true
	if (it != chunkList.end()) {
		file.seekg(cameraDataOffset+sizeof(uint32_t) + chunkList.size() * sizeof(long) + chunkDataOffset * pos);
		file.read((char*)chunk->blocks, chunkDataOffset);
		return true;
	}
	//couldn't find chunk in our world file
	else {
		return false;
	}
}


void worldSaver::writeChunk(const Chunk &chunk) {
	
	long hash = World::cantorHash(chunk.x, chunk.z);
	auto it = std::find(chunkList.begin(), chunkList.end(), hash);
	auto pos = std::distance(chunkList.begin(), it);
	if (it != chunkList.end()) {
		//we found our chunk, go to it and write the new one
		file.seekp(cameraDataOffset+sizeof(uint32_t) + chunkList.size() * sizeof(long) + chunkDataOffset * pos);
		file.write((char*)chunk.blocks, chunkDataOffset);
		return;
	}
	else {
		//we couldn't find our chunk, add it to the header then append it to the end

		//update chunk list
		//have to do this buffer workaround cause we can't append in the middle of a file
		//make buffer
		auto chunkDataSize = chunkList.size() * CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_HEIGHT;
		std::vector<char> tempBuff(4+chunkDataSize);
		std::memcpy(tempBuff.data(), &hash, sizeof(long));
		file.seekg(cameraDataOffset+4+chunkList.size()*4);
		file.read(tempBuff.data() + 4, chunkDataSize);

		//write new buffer
		file.seekp(cameraDataOffset+4+chunkList.size()*4);
		file.write(tempBuff.data(), tempBuff.size());
		chunkList.push_back(hash);

		//update number of chunks
		uint32_t numChunks = chunkList.size();
		file.seekp(cameraDataOffset);
		file.write((char*)&numChunks, sizeof(uint32_t));

		//write chunk itself to end of file
		file.seekp(0,std::ios::end);
		file.write((char*)chunk.blocks, chunkDataOffset);

		return;

	}

}
