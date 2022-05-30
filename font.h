#pragma once
#include <iostream>
#include <map>
#include "glm/glm.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader.h"

struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};

//this class was thoughtlessly ripped off from learnopengl.com
class TextWriter {
public:
    unsigned int VAO, VBO;

    //Shader s;
	Shader s = Shader("../shaders/vert_text.glsl", "D:/Programming/Minecraft-gl/Opengl2/shaders/frag_text.glsl");
    glm::mat4 projection = glm::ortho(0.0f, 1200.0f, 0.0f, 1000.0f);

    std::map<char, Character> Characters;

    TextWriter();
	

    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

    ~TextWriter()
    {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
    }
};