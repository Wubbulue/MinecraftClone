#ifndef LINE_HEADER
#define LINE_HEADER

#include "VAO.h"
#include "VBO.h"
#include "shader.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glad/glad.h>
#include <memory>

class Line {
public:
    glm::vec3 startPoint;
    glm::vec3 endPoint;
    glm::vec3 lineColor;

    //fancy model matrix to place line, only needs to be calculated once
    glm::mat4 modelMatrix;

    Line(glm::vec3 start, glm::vec3 end, glm::vec3 color);

    void setColor(glm::vec3 color);


};


#endif // !LINE_HEADER
