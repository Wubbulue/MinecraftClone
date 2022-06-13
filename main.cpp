#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Player.h"
#include <chrono>
#include <memory>
#include "line.h"
#include "VBO.h"
#include "VAO.h"
#include "minecraft.h"
#include "PerlinNoise.h"
#include "shader.h"
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "font.h"
#include "Colors.h"
#include "save.h"



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



//callled on window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
unsigned int SCR_WIDTH = 1200;
unsigned int SCR_HEIGHT = 1000;

float mixAmount = 0.5f;
float fov = 45.0f;
float camHeight = 0.0f;
const float mouseSensitivity = 0.1f;

bool renderDebugInfo = true;
bool shouldFrustumCull = true;
bool frustumCullUsingCube = true;


Player player;
std::shared_ptr<worldSaver> saver;
std::unique_ptr<ChunkManager> chunkManager;


//Chunk chunk(123489u,0,0);
//World world(123489u);
World world(153389u);
bool wireframe = false;

const float lineLength = 30.0f;
std::vector<Line> cameraLines;
std::vector<Line> blockLines;

float cubeRadius;



float outlineVerts[] = {

	0.0f,0.0f,0.0f, //0
	0.0f,0.0f,1.0f, //1
	0.0f,1.0f,0.0f, //2
	0.0f,1.0f,1.0f, //3
	1.0f,0.0f,0.0f, //4
	1.0f,0.0f,1.0f, //5
	1.0f,1.0f,0.0f, //6
	1.0f,1.0f,1.0f, //7


};

uint8_t bLineIndices[] = {
	0,1,
	0,2,
	0,4,
	1,3,
	1,5,
	2,6,
	2,3,
	3,7,
	4,5,
	4,6,
	5,7,
	6,7,
};

void drawCamFrustum() {
	Frustum camFrustum = createFrustumFromCamera(player.cam, float(SCR_WIDTH) / float(SCR_HEIGHT));
	cameraLines.emplace_back(Line(player.cam.position, player.cam.position + camFrustum.rightFace.normal, COLORS::red));
	cameraLines.emplace_back(Line(player.cam.position, player.cam.position + camFrustum.leftFace.normal, COLORS::blue));
	cameraLines.emplace_back(Line(player.cam.position, player.cam.position + camFrustum.topFace.normal, COLORS::purple));
	cameraLines.emplace_back(Line(player.cam.position, player.cam.position + camFrustum.bottomFace.normal, COLORS::yellow));
	cameraLines.emplace_back(Line(player.cam.position, player.cam.position + camFrustum.nearFace.normal, COLORS::cyan));
	cameraLines.emplace_back(Line(player.cam.position, player.cam.position + camFrustum.farFace.normal, COLORS::white));

}

void updateBlockPlayerLookingAt() {

	BlockPosition oldPos = player.blockLookingAt;
	if (world.findFirstSolid(Ray(player.cam.position, player.cam.direction), 30.0f, player.blockLookingAt)) {
		player.isLookingAtBlock = true;
		if (oldPos != player.blockLookingAt) {
			//TODO: at times this is rendering too many lines, maybe check adjecent blocks for which lines to render
			//TODO: this block outline suffers from z fighting
			blockLines.clear();

			float fx = float(player.blockLookingAt.x);
			float fy = float(player.blockLookingAt.y);
			float fz = float(player.blockLookingAt.z);

			//12 lines in a cube loop
			for (int i = 0; i < 12; i++) {
				//line indexes
				uint8_t j1 = bLineIndices[i * 2];
				uint8_t j2 = bLineIndices[(i * 2) + 1];
				j1 *= 3;
				j2 *= 3;

				glm::vec3 start(fx + outlineVerts[j1], fy + outlineVerts[j1 + 1], fz + outlineVerts[j1 + 2]);
				glm::vec3 end(fx + outlineVerts[j2], fy + outlineVerts[j2 + 1], fz + outlineVerts[j2 + 2]);

				blockLines.emplace_back(Line(start, end, glm::vec3(0.8f, 0.2f, 0.2f)));

			}
		}
	}
	else {
		player.isLookingAtBlock = false;
		blockLines.clear();
	}
}

void checkCompilation(const char* shaderName, unsigned int shader) {

	int  success;
	char infoLog[512];


	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::" << shaderName << "::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		glm::vec3 start = player.cam.position;
		glm::vec3 end = start + (player.cam.direction * lineLength);
		cameraLines.emplace_back(Line(start, end, glm::vec3(1.0f, 0.0f, 0.0f)));

		auto t1 = std::chrono::high_resolution_clock::now();
		Ray ray(start, player.cam.direction);
		world.mineHoleCast(ray, 30.0f);
		auto t2 = std::chrono::high_resolution_clock::now();

		// floating-point duration: no duration_cast needed
		std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

		// integral duration: requires duration_cast
		auto int_ms = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

		std::cout << "Took " << int_ms.count() << " microseconds to elimnate blocks" << std::endl;



	}
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {

	static float lastX = SCR_WIDTH / 2;
	static float lastY = SCR_HEIGHT / 2;

	static bool firstMouse = true;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;
	player.cam.ProcessMouseMovement(xoffset, yoffset);
	updateBlockPlayerLookingAt();


}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	player.cam.processSroll(float(yoffset));
}


//will only get called when a key is pressed
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_N && action == GLFW_PRESS) //regenerate chunks
	{
		world.regenerate();
	}
	else if (key == GLFW_KEY_T && action == GLFW_PRESS) //toggle wireframes
	{
		wireframe = !wireframe;
		if (wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) //Print position
	{
		printf("Position:%f,%f,%f \n", player.cam.position.x, player.cam.position.y, player.cam.position.z);
	}
	else if (key == GLFW_KEY_C && action == GLFW_PRESS) //Clear lines
	{
		world.addChunk(1, 0);

	}
	else if (key == GLFW_KEY_F3 && action == GLFW_PRESS) //Render debug info
	{
		renderDebugInfo = !renderDebugInfo;
	}
	else if (key == GLFW_KEY_F && action == GLFW_PRESS) //Frustum cull
	{
		shouldFrustumCull = !shouldFrustumCull;
		if (shouldFrustumCull) {
			printf("Frustum culling enabled\n");
		}
		else {
			printf("Frustum culling disabled\n");
		}
		//drawCamFrustum();
	}
	else if (key == GLFW_KEY_K && action == GLFW_PRESS) //Write chunk 0,0
	{

		//auto r = world.renderDistance;
		//for (int i = r; i >= -r; i--) {
		//	for (int j = r; j >= -r; j--) {
		//		Chunk* chunk = world.getChunk(i, j);
		//		saver->writeChunk(*chunk);
		//	}
		//}

	}
	else if (key == GLFW_KEY_L && action == GLFW_PRESS) //Load chunk 0,0
	{

		//auto r = world.renderDistance;
		//for (int i = r; i >= -r; i--) {
		//	for (int j = r; j >= -r; j--) {
		//		Chunk* chunk = world.getChunk(i, j);
		//		saver->tryFillChunk(chunk);
		//	}
		//}

	}
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
		saver->writePosition();
		saver->closeWorld();
	}
}

//relatively rotates something around a point
void rotateAboutPoint(glm::mat4& mat, float rotationAmount, float xOffset, float yOffset) {
	mat = glm::translate(mat, glm::vec3(xOffset, yOffset, 0.0f));
	mat = glm::rotate(mat, rotationAmount, glm::vec3(0.0f, 0.0f, 1.0f));
	mat = glm::translate(mat, glm::vec3(-xOffset, -yOffset, 0.0f));
}


int main()
{
	saver = std::make_shared<worldSaver>("../saves/save.world", &player);
	chunkManager = std::make_unique<ChunkManager>(saver.get(), &player, &world);
	chunkManager->initWorld();

	

	cubeRadius = 1.0f / cos(glm::radians(45.0f));


	//world.addChunk(0, 0);
	//world.addChunk(-1, 0);
	//world.addChunk(0, -1);
	//world.addChunk(-1, -1);




	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//set input callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);



	//enable z buffer!
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_NEVER);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



	Shader lineShader("../shaders/vert_line.glsl", "../shaders/frag_line.glsl","line shader");

	//create a shader program from a vert and frag path
	Shader shaderTexture("../shaders/vert_texture.glsl", "../shaders/frag_texture.glsl","texture shader");
	shaderTexture.use();

	Shader diffuseShader("../shaders/vert_diffuse.glsl", "../shaders/frag_diffuse.glsl", "diffuse shader");

	unsigned int atlasTexture;
	{
		//--------------------------TEXTURES---------------------------------------------------


		// load and create a texture 
		// -------------------------
		// Dirt
		// ---------
		glGenTextures(1, &atlasTexture);
		glBindTexture(GL_TEXTURE_2D, atlasTexture);

		// set the texture wrapping parameters (after generating texture i think?)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
		unsigned char* data = stbi_load("../textures/default_texture.png", &width, &height, &nrChannels, 0);
		//unsigned char* data = stbi_load("D:/Programming/Minecraft-gl/Opengl2/textures/dirt.jpg", &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);

		//-------------------------------------------------------------------------------------------
	}



	//--------------------GEOMETERY AND STUFF-------------------------------------------
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	float lineVerts[] = {
		0.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,

	};

	//TODO: use classes for these
	unsigned int cubeVBO,dirtVBO,stoneVBO, dirtVAO, stoneVAO, lineVAO, lineVBO;
	glGenVertexArrays(1, &dirtVAO); // we can also generate multiple VAOs or buffers at the same time
	glGenVertexArrays(1, &stoneVAO); // we can also generate multiple VAOs or buffers at the same time
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1,&dirtVBO);
	glGenBuffers(1,&stoneVBO);

	glGenVertexArrays(1, &lineVAO); // we can also generate multiple VAOs or buffers at the same time
	glGenBuffers(1, &lineVBO);
	

	// first line setup
	// --------------------
	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVerts), lineVerts, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//--------------------------------------------------------------------------

	//TODO: probably there is a cleaner way to do this

	//--------------------------dirt-------------------------------------
	glBindVertexArray(dirtVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, dirtVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(dirtFaces), dirtFaces, GL_STATIC_DRAW);

	glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE,  0, (void*)(0));
	glEnableVertexAttribArray(2);
	//--------------------------dirt-------------------------------------

	//--------------------------stone-------------------------------------
	glBindVertexArray(stoneVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, stoneVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(stoneFaces), stoneFaces, GL_STATIC_DRAW);

	glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE,  0, (void*)(0));
	glEnableVertexAttribArray(2);
	//--------------------------stone-------------------------------------


	//COORDINATE TRANSFORMATIONS
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);


	TextWriter fontWriter;


	// render loop
	// -----------
	float lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		//calc frame rate
		float timeValue = glfwGetTime();
		float elapsedTime = timeValue - lastTime;
		float frameRate = 1 / elapsedTime;
		lastTime = timeValue;



		if (player.checkPosition(window, elapsedTime)) {
			//player could be looking at new block after camera movement
			updateBlockPlayerLookingAt();
			chunkManager->checkNewChunk();
		}

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, atlasTexture);



		//update coordiante transformations
		projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(player.cam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, player.cam.near, player.cam.far);

		view = player.cam.view();

		diffuseShader.use();
		diffuseShader.setMat4("model", model);
		diffuseShader.setMat4("view", view);
		diffuseShader.setMat4("projection", projection);

		shaderTexture.use();
		shaderTexture.setMat4("model", model);
		shaderTexture.setMat4("view", view);
		shaderTexture.setMat4("projection", projection);


		glBindVertexArray(dirtVAO);


		auto toRender = world.getBlocksToRender(player.chunkX, player.chunkZ);
		int blocksCulled = 0;
		Frustum camFrustum = createFrustumFromCamera(player.cam, float(SCR_WIDTH) / float(SCR_HEIGHT));


		int chunkNum = 0,numChunkX=0,numChunkZ=0;
		for (int i = player.chunkX - world.renderDistance; i < player.chunkX + world.renderDistance+1; i++) {

			numChunkZ = 0;
			for (int j = player.chunkZ - world.renderDistance; j < player.chunkZ + world.renderDistance+1; j++) {


				float offsetX = i * CHUNK_LENGTH;
				float offsetZ = j * CHUNK_LENGTH;

				for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {

					for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
						for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {

							auto blockType = toRender[world.customIndex(x+(numChunkX*CHUNK_LENGTH),z+(numChunkZ*CHUNK_LENGTH),y)].type;
							//if (blockType == BlockTypes::Air || !world.isBlockAdjacentToAir(pos)) {
							if (blockType == BlockTypes::Air) {
								continue;
							}

							if (shouldFrustumCull) {

								glm::vec3 center(float(x + 0.5f) + offsetX, float(y + 0.5f), float(z + 0.5f) + offsetZ);

								SquareAABB square(center, 0.5f);
								bool isOnFurstum = square.isOnFrustum(camFrustum);

								if (!isOnFurstum) {
									blocksCulled++;
									continue;
								}

							}

							model = glm::mat4(1.0f);


							//offset by half voxel for center
							model = glm::translate(model, glm::vec3(float(x + 0.5f) + offsetX, float(y + 0.5f), float(z + 0.5f) + offsetZ));

							if (player.isLookingAtBlock) {
								auto p = player.blockLookingAt;
								p.x -= i * CHUNK_LENGTH;
								p.z -= j * CHUNK_LENGTH;
								if ((p.x == x) && (p.y == y) && (p.z == z)) {
									diffuseShader.use();
									diffuseShader.setMat4("model", model);
									glDrawArrays(GL_TRIANGLES, 0, 36);
									shaderTexture.use();
									continue;
								}
							}

							shaderTexture.setMat4("model", model);

							if (blockType== BlockTypes::Dirt) {
								glBindVertexArray(dirtVAO);
							}
							else if (blockType == BlockTypes::Stone) {
								glBindVertexArray(stoneVAO);
							}

							glDrawArrays(GL_TRIANGLES, 0, 36);


						}
					}
				}
				chunkNum++;
				numChunkZ++;

			}

			numChunkX++;

		}





		glBindVertexArray(lineVAO);

		//should all lines be rendered like this? idk TODO
		glm::mat4 MVP = projection * view * model;
		lineShader.use();
		lineShader.setMat4("MVP", MVP);





		for (auto const& line : cameraLines) {
			lineShader.setVec3("color", line.lineColor);
			MVP = projection * view * line.modelMatrix;
			lineShader.setMat4("MVP", MVP);
			glDrawArrays(GL_LINES, 0, 2);
		}


		for (auto const& line : blockLines) {
			lineShader.setVec3("color", line.lineColor);
			MVP = projection * view * line.modelMatrix;
			lineShader.setMat4("MVP", MVP);
			glDrawArrays(GL_LINES, 0, 2);
		}


		if (renderDebugInfo) {
			fontWriter.RenderText(std::to_string(frameRate), 25.0f, SCR_HEIGHT - 100, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
			std::string posString = std::to_string(player.cam.position.x) + ", " + std::to_string(player.cam.position.y) + ", " + std::to_string(player.cam.position.z);
			fontWriter.RenderText(posString, 25.0f, SCR_HEIGHT - 150, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
			std::string culledString = "Blocks culled: " + std::to_string(blocksCulled);
			fontWriter.RenderText(culledString, 25.0f, SCR_HEIGHT - 200, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));

			std::string blockString;
			if (player.isLookingAtBlock) {
				auto b = player.blockLookingAt;
				std::string blockTypeString = blockTypeToString(world.getBlock(b)->type);
				blockString = "Player is looking at: " + blockTypeString + ", in position " + std::to_string(b.x) + "," + std::to_string(b.y) + "," + std::to_string(b.z);
			}
			else {
				blockString = "Player is not looking at block";
			}
			fontWriter.RenderText(blockString, 25.0f, SCR_HEIGHT - 250, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));

		}



		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &dirtVAO);
	glDeleteVertexArrays(1, &stoneVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &dirtVBO);
	glDeleteBuffers(1, &stoneVBO);

	glDeleteVertexArrays(1, &lineVAO);
	glDeleteBuffers(1, &lineVBO);
	//glDeleteBuffers(1, &EBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	SCR_HEIGHT = height;
	SCR_WIDTH = width;
	glViewport(0, 0, width, height);
}