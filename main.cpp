#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

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
#include "Timer.h"
#include "ThreadPool.h"
#include <atomic>



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"





//callled on window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;

float mixAmount = 0.5f;
float fov = 45.0f;
float camHeight = 0.0f;
// unsigned int lightLevel = 8;
const float mouseSensitivity = 0.1f;

bool renderDebugInfo = true;
bool lockFrustum = false;
Frustum oldFrustum;


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

//need access to this shader on user input callback, thus put it here
std::unique_ptr<Shader> shaderTexture;

BlockType blockToPlace = BlockTypes::Stone;


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

		Ray ray(start, player.cam.direction);
		BlockPosition pos;

		if (world.findFirstSolid(ray, 30.0f, pos)) {
			auto block = world.getBlock(pos);
			block->type = BlockTypes::Air;
			world.renderBlocksDirty = true;
			world.frustumCullDirty = true;
			world.lightDirty = true;
			world.vboDirty = true;
			auto chunk = world.getChunkContainingBlock(pos);
			saver->writeChunk(*chunk);
		}

	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {

		Ray ray(player.cam.position, player.cam.direction);
		BlockPosition pos;

		if (world.getPlaceBlock(ray, 30.0f, pos)) {
			auto block = world.getBlock(pos);
			block->type = blockToPlace;
			world.renderBlocksDirty = true;
			world.frustumCullDirty = true;
			world.lightDirty = true;
			world.vboDirty = true;
			auto chunk = world.getChunkContainingBlock(pos);
			saver->writeChunk(*chunk);
		}


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
	world.frustumCullDirty = true;
	world.vboDirty = true;
	updateBlockPlayerLookingAt();


}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	player.cam.processSroll(float(yoffset));
}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

//will only get called when a key is pressed
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_N && action == GLFW_PRESS) //regenerate chunks
	{
		world.regenerate();
		world.renderBlocksDirty = true;
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
	else if (key == GLFW_KEY_F3 && action == GLFW_PRESS) //Render debug info
	{
		renderDebugInfo = !renderDebugInfo;
	}
	else if (key == GLFW_KEY_F && action == GLFW_PRESS) //Frustum cull
	{
		lockFrustum = !lockFrustum;
		if (lockFrustum) {
			oldFrustum = createFrustumFromCamera(player.cam, float(SCR_WIDTH) / float(SCR_HEIGHT));
		}
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) //Write chunk 0,0
	{
		//TODO: make this hot reload all shaders
		shaderTexture->hotReload();
	}
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		saver->writePosition();
		saver->closeWorld();
		glfwSetWindowShouldClose(window, true);
	}
	//move block place type left
	else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		if (blockToPlace == 1) {
			blockToPlace = BlockTypes::blockTypeStrings.size() - 1;
		}
		else {
			blockToPlace--;
		}
	}
	//move block place type right
	else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		if (blockToPlace == (BlockTypes::blockTypeStrings.size() - 1)) {
			blockToPlace = 1;
		}
		else {
			blockToPlace++;
		}
	}
	//Toggle between walk mode and fly mode
	else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		player.walkMode = !player.walkMode;
		if (!player.walkMode) {
			player.physics_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		}
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

	glfwSetErrorCallback(glfw_error_callback);

	saver = std::make_shared<worldSaver>("../save.world", &player);
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


	const char* glsl_version = "#version 330";
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	world.initOpenGL();








	Shader lineShader("../shaders/vert_line.glsl", "../shaders/frag_line.glsl", "line shader");

	shaderTexture = std::make_unique<Shader>("../shaders/vert_texture.glsl", "../shaders/frag_texture.glsl", "texture shader");
	//create a shader program from a vert and frag path
	shaderTexture->use();
	// shaderTexture->setUint("lightLevel", lightLevel);

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
	unsigned int lineVAO, lineVBO;

	glGenVertexArrays(1, &lineVAO);
	glGenBuffers(1, &lineVBO);


	// first line setup
	// --------------------
	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVerts), lineVerts, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);




	//COORDINATE TRANSFORMATIONS
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);


	TextWriter fontWriter;

	colorUnpacked c = { 2,7,12,5 };
	colorPacked p = World::packColor(c);
	colorUnpacked after = World::unpackColor(p);



	std::vector<float> frameRates;
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
		frameRates.push_back(frameRate);

		player.computeMovementVelocity(window);
		if (player.tick(elapsedTime)) {
			//player could be looking at new block after camera movement
			world.frustumCullDirty = true;
			world.vboDirty = true;
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

		shaderTexture->use();
		shaderTexture->setMat4("model", model);
		shaderTexture->setMat4("view", view);
		shaderTexture->setMat4("projection", projection);






		Frustum camFrustum = lockFrustum ? oldFrustum : createFrustumFromCamera(player.cam, float(SCR_WIDTH) / float(SCR_HEIGHT));
		world.getBlocksToRenderThreaded(player.chunkX, player.chunkZ, camFrustum);







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
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			//ImGui::ImVec2 LastValidMousePos(3,4);

			ImGui::SetNextWindowPos({ 20,15 });
			bool open = false;
			bool* p_open = &open;
			ImGui::Begin("Debug Info", p_open, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Position: x: %f, y: %f, z: %f", player.cam.position.x, player.cam.position.y, player.cam.position.z);
			ImGui::Text("Block type to place: %s    (Change with Q and E)", BlockTypes::blockTypeToString(blockToPlace).c_str());
			if (player.isLookingAtBlock) {
				auto b = player.blockLookingAt;
				std::string blockTypeString = BlockTypes::blockTypeToString(world.getBlock(b)->type);
				ImGui::Text("Player is looking at %s in position %d,%d,%d", blockTypeString.c_str(), b.x, b.y, b.z);
			}
			else {
				ImGui::Text("Player is not looking at block");
			}
			ImGui::Text(player.walkMode ? "Walk mode" : "Fly mode");
			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &lineVAO);
	glDeleteBuffers(1, &lineVBO);

	world.cleanOpenGL();
	//glDeleteBuffers(1, &EBO);


	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();

	frameRates.erase(frameRates.begin());
	auto avgFrameRate = std::accumulate(frameRates.begin(), frameRates.end(), 0) / frameRates.size();
	printf("Average frame rate of %d frames was %f", int(frameRates.size()), avgFrameRate);

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
