#include "line.h"




Line::Line(glm::vec3 start, glm::vec3 end, glm::vec3 color) {


	startPoint = start;
	endPoint = end;
	lineColor = color;

	//initialize model matrix
	auto rotation = glm::rotation(glm::vec3(1.0f, 0.0f, 0.0f), glm::normalize(endPoint - startPoint));
	auto rotationMat = glm::toMat4(rotation);
	float length = glm::length(endPoint - startPoint);
	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, startPoint);
	//model = model * rotationMat;
	modelMatrix *= rotationMat;
	modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 1.0f, 1.0f));



}


void Line::setColor(glm::vec3 color) {
	lineColor = color;
}

