#include "line.h"




Line::Line(glm::vec3 start, glm::vec3 end, glm::vec3 color) {

    printf("\nLine constructed %p\n", this);

	MVP = glm::mat4(1.0f);
	startPoint = start;
	endPoint = end;
	lineColor = color;


	vertices[0] = start.x;
	vertices[1] = start.y;
	vertices[2] = start.z;

	vertices[3] = end.x;
	vertices[4] = end.y;
	vertices[5] = end.z;
	//vbo = new VBO(vertices, sizeof(vertices));
	vbo = std::make_shared<VBO>(vertices, sizeof(vertices));

	vao.Bind();
	vao.LinkAttrib(*vbo, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

}

    void Line::setMVP(glm::mat4 mvp) {
        MVP = mvp;
    }

    void Line::setColor(glm::vec3 color) {
        lineColor = color;
    }

    int Line::draw() {
        shader.use();

		int MVPLoc = glGetUniformLocation(shader.ID, "MVP");
		glUniformMatrix4fv(MVPLoc, 1, GL_FALSE, glm::value_ptr(MVP));

        shader.setMat4("MVP", MVP);
        shader.setVec3("color", lineColor);


        vao.Bind();
        glDrawArrays(GL_LINES, 0, 2);
        return 1;
    }
