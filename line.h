#ifndef LINE_HEADER
#define LINE_HEADER

#include "VAO.h"
#include "VBO.h"
#include "shader.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glad/glad.h>
#include <memory>

class Line {
public:
    Shader shader = Shader("C:/Programming_projects/Open-GL/shaders/vert_line.glsl", "C:/Programming_projects/Open-GL/shaders/frag_line.glsl");
    VAO vao;
    //VBO *vbo;
    std::shared_ptr<VBO> vbo;
    float vertices[6];
    glm::vec3 startPoint;
    glm::vec3 endPoint;
    glm::vec3 lineColor;
    glm::mat4 MVP;

    Line(glm::vec3 start, glm::vec3 end, glm::vec3 color);

    void setMVP(glm::mat4 mvp);

    void setColor(glm::vec3 color);

    int draw();

    ~Line() {

        vao.Delete();
        vbo->Delete();
        printf("Line destructed %p\n", this);
        //delete(vbo);
    }
};


#endif // !LINE_HEADER
