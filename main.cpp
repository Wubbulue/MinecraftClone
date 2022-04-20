#include "Player.h"
#include <chrono>
#include <memory>
#include "line.h"
#include "VBO.h"
#include "VAO.h"
#include "minecraft.h"
#include "PerlinNoise.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Camera.h"
#include "font.h"



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


Player player;
Camera camera;

//Chunk chunk(123489u,0,0);
World world(123489u);
bool wireframe = false;

const float lineLength = 30.0f;
std::vector<Line> cameraLines;
std::vector<Line> blockLines;



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

void updateBlockPlayerLookingAt() {

	BlockPosition oldPos = player.blockLookingAt;
	if (world.findFirstSolid(Ray(camera.position, camera.direction), 30.0f,player.blockLookingAt)) {
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

				glm::vec3 start(fx+outlineVerts[j1],fy+outlineVerts[j1+1],fz+outlineVerts[j1+2]);
				glm::vec3 end(fx+outlineVerts[j2],fy+outlineVerts[j2+1],fz+outlineVerts[j2+2]);

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
		glm::vec3 start = camera.position;
		glm::vec3 end = start+(camera.direction * lineLength);
		cameraLines.emplace_back(Line(start, end, glm::vec3(1.0f, 0.0f, 0.0f)));

		auto t1 = std::chrono::high_resolution_clock::now();
		Ray ray(start,camera.direction);
		world.mineHoleCast(ray,30.0f);
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
	camera.ProcessMouseMovement(xoffset, yoffset);
	updateBlockPlayerLookingAt();
	

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.processSroll(float(yoffset));
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
		printf("Position:%f,%f,%f \n",camera.position.x,camera.position.y,camera.position.z);
	}
	else if (key == GLFW_KEY_C && action == GLFW_PRESS) //Clear lines
	{
		cameraLines.clear();
	}
	else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

//relatively rotates something around a point
void rotateAboutPoint(glm::mat4 &mat,float rotationAmount, float xOffset, float yOffset) {
		mat = glm::translate(mat, glm::vec3(xOffset,yOffset, 0.0f));
		mat = glm::rotate(mat, rotationAmount, glm::vec3(0.0f, 0.0f, 1.0f));
		mat = glm::translate(mat, glm::vec3(-xOffset,-yOffset, 0.0f));
}


int main()
{



	world.addChunk(0, 0);
	//world.addChunk(-1, 0);
	//world.addChunk(1, 0);
	//world.addChunk(0, 1);
	//world.addChunk(1, 1);


	camera.position.y = 50.0f;


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
	glfwSetMouseButtonCallback(window,mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);



	//enable z buffer!
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_NEVER);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



	Shader lineShader("C:/Programming_projects/Open-GL/shaders/vert_line.glsl", "C:/Programming_projects/Open-GL/shaders/frag_line.glsl");

	//create a shader program from a vert and frag path
	Shader shaderTexture("C:/Programming_projects/Open-GL/shaders/vert_texture.glsl", "C:/Programming_projects/Open-GL/shaders/frag_texture.glsl");
	shaderTexture.use();

	Shader diffuseShader("C:/Programming_projects/Open-GL/shaders/vert_diffuse.glsl", "C:/Programming_projects/Open-GL/shaders/frag_diffuse.glsl");

	unsigned int dirtTexture, stoneTexture;

	{
		//--------------------------TEXTURES---------------------------------------------------


		// load and create a texture 
		// -------------------------
		// Dirt
		// ---------
		glGenTextures(1, &dirtTexture);
		glBindTexture(GL_TEXTURE_2D, dirtTexture);

		// set the texture wrapping parameters (after generating texture i think?)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
		unsigned char* data = stbi_load("C:/Programming_projects/Open-GL/textures/grass_block_side.png", &width, &height, &nrChannels, 0);
		//unsigned char* data = stbi_load("C:/Programming_projects/Open-GL/textures/dirt.jpg", &width, &height, &nrChannels, 0);
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

		// Stone
		// ---------
		glGenTextures(1, &stoneTexture);
		glBindTexture(GL_TEXTURE_2D, stoneTexture);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// load image, create texture and generate mipmaps
		data = stbi_load("C:/Programming_projects/Open-GL/textures/stone.jpg", &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);

		// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
		// -------------------------------------------------------------------------------------------
		shaderTexture.use(); // don't forget to activate/use the shader before setting uniforms!

		// either set it manually like so:
		shaderTexture.setInt("texture", 0);
		//-------------------------------------------------------------------------------------------
	}



	//--------------------GEOMETERY AND STUFF-------------------------------------------
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	float lineVerts[] = {
		0.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,

	};

	float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	glm::vec3 cubePositions[] = {
	glm::vec3(0.0f,  0.0f,  0.0f),
	glm::vec3(2.0f,  5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f,  3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f,  2.0f, -2.5f),
	glm::vec3(1.5f,  0.2f, -1.5f),
	glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	//TODO: use classes for these
	unsigned int VBO, VAO,lineVAO,lineVBO;
	glGenVertexArrays(1, &VAO); // we can also generate multiple VAOs or buffers at the same time
	glGenBuffers(1, &VBO);

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

	// first triangle setup
	// --------------------
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	//COORDINATE TRANSFORMATIONS
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	

	TextWriter fontWriter;

	//chunk.empty();
	//chunk.populateBlocks();

	// render loop
	// -----------
	float lastTime=glfwGetTime();
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
		fontWriter.RenderText(std::to_string(frameRate), 25.0f, SCR_HEIGHT - 100, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));

		if (camera.moveCamera(window, elapsedTime)) {
			//player could be looking at new block after camera movement
			updateBlockPlayerLookingAt();
		}

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, dirtTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, stoneTexture);



		//update coordiante transformations
		projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		view = camera.view();

		diffuseShader.use();
		diffuseShader.setMat4("model", model);
		diffuseShader.setMat4("view", view);
		diffuseShader.setMat4("projection", projection);

		shaderTexture.use();
		shaderTexture.setMat4("model", model);
		shaderTexture.setMat4("view", view);
		shaderTexture.setMat4("projection", projection);


		glBindVertexArray(VAO);

		//TODO: implement proper instancing
		for (auto& [key, chunk] : world.chunks)
		{

			float offsetX = chunk.x * CHUNK_LENGTH;
			float offsetZ = chunk.z * CHUNK_LENGTH;
			for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {

				for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
					for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {

						auto blockType = chunk.blocks[index(x, z, y)].type;
						if (blockType == BlockType::Air || !chunk.isBlockAdjacentToAir(x, y, z)) {
							continue;
						}
						model = glm::mat4(1.0f);


						//offset by half voxel for center
						model = glm::translate(model, glm::vec3(float(x + 0.5f)+offsetX, float(y + 0.5f), float(z + 0.5f)+offsetZ));

						if (player.isLookingAtBlock) {
							auto p = player.blockLookingAt;
							p.x -= chunk.x * CHUNK_LENGTH;
							p.z -= chunk.z * CHUNK_LENGTH;
							if ((p.x == x) && (p.y == y) && (p.z == z)) {
								diffuseShader.use();
								diffuseShader.setMat4("model", model);
								glDrawArrays(GL_TRIANGLES, 0, 36);
								shaderTexture.use();
								continue;
							}
						}

						shaderTexture.setMat4("model", model);

						if (blockType == BlockType::Dirt) {
							shaderTexture.setInt("texture", 0);
						}
						else if (blockType == BlockType::Stone) {
							shaderTexture.setInt("texture", 1);
						}

						glDrawArrays(GL_TRIANGLES, 0, 36);
					}
				}
			}
		}





		glBindVertexArray(lineVAO);

		//should all lines be rendered like this? idk TODO
		glm::mat4 MVP = projection * view * model;
		lineShader.use();
		lineShader.setMat4("MVP", MVP);





		for (auto const &line:cameraLines) {
			lineShader.setVec3("color", line.lineColor);
			MVP = projection * view * line.modelMatrix;
			lineShader.setMat4("MVP", MVP);
			glDrawArrays(GL_LINES, 0, 2);
		}


		for (auto const &line:blockLines) {
			lineShader.setVec3("color", line.lineColor);
			MVP = projection * view * line.modelMatrix;
			lineShader.setMat4("MVP", MVP);
			glDrawArrays(GL_LINES, 0, 2);
		}



		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

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