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

		//read actual chunks in
		//for (int i = 0; i < numChunks; i++) {
		//	file.read((char*)&chunkList[i], sizeof(long));
		//}

		cam->updateCameraVectors();

	}

}

void worldSaver::writeWorld() {

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
	if (it != chunkList.end()) {
		file.seekg(cameraDataOffset+sizeof(uint32_t) + chunkList.size() * sizeof(long) + chunkDataOffset * pos);
		file.read((char*)chunk->blocks, chunkDataOffset);
		return true;
	}
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


		//do this before append mode, write chunk number
		uint32_t numChunks = chunkList.size()+1;
		file.seekp(cameraDataOffset);
		file.write((char*)&numChunks, sizeof(uint32_t));

		//open file in append mode
		file.close();
		file.open(filename.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::app);

		//write to chunk list header first
		file.seekp(cameraDataOffset+sizeof(uint32_t) + chunkList.size() * sizeof(long));
		file.write((char*)&hash, sizeof(long));
		chunkList.push_back(hash);

		//write chunk itself to end of file
		file.seekp(0,std::ios::end);
		file.write((char*)chunk.blocks, chunkDataOffset);

		//reopen file in overwrite mode
		file.close();
		file.open(filename.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary);
		return;

	}

}
