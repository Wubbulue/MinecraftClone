#include "geometery.h"

bool rayIntersect(glm::vec3 rayOrigin, glm::vec3 rayVector, Triangle *inTriangle, glm::vec3 &outIntersectionPoint) {
	const float EPSILON = 0.0000001;
	glm::vec3 vertex0 = inTriangle->points[0];
	glm::vec3 vertex1 = inTriangle->points[1];
	glm::vec3 vertex2 = inTriangle->points[2];
	glm::vec3 edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0;
	h = glm::cross(rayVector, edge2);
	a = glm::dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return false;    // This ray is parallel to this triangle.
	f = 1.0 / a;
	s = rayOrigin - vertex0;
	//u = f * s.dotProduct(h);
	u = f * glm::dot(s,h);
	if (u < 0.0 || u > 1.0)
		return false;
	//q = s.crossProduct(edge1);
	q = glm::cross(s,edge1);
	//v = f * rayVector.dotProduct(q);
	v = f * glm::dot(rayVector,q);
	if (v < 0.0 || u + v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.
	//float t = f * edge2.dotProduct(q);
	float t = f * glm::dot(edge2,q);
	if (t > EPSILON) // ray intersection
	{
		outIntersectionPoint = rayOrigin + rayVector * t;
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;

}
