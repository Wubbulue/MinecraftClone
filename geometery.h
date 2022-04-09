#ifndef GEOMETERY_HEADER
#define GEOMETERY_HEADER

#include "glm/glm.hpp"
#include <algorithm>


struct Triangle {
	glm::vec3 points[3];
};

//typedef glm::vec3 Triangle[3];

//Möller–Trumbore intersection algorithm from wikipedia to calculate intersection of ray and triangle
bool rayIntersect(glm::vec3 rayOrigin, glm::vec3 rayVector, Triangle* inTriangle, glm::vec3& outIntersectionPoint);

class Ray
{
public:
    Ray(const glm::vec3& orig, const glm::vec3& dir) : orig(orig), dir(dir)
    {
        invdir = glm::vec3(1.0f,1.0f,1.0f) / dir;
        sign[0] = (invdir.x < 0);
        sign[1] = (invdir.y < 0);
        sign[2] = (invdir.z < 0);
    }
    glm::vec3 orig, dir;       // ray orig and dir 
    glm::vec3 invdir;
    int sign[3];
};


//axis alligned bounding box
class Box3
{
public:
	Box3(const glm::vec3& vmin, const glm::vec3& vmax)
	{
		bounds[0] = vmin;
		bounds[1] = vmax;
	}
	glm::vec3 bounds[2];

    //calculate intersection of box and triangle
    bool intersect(const Ray& r, glm::vec3& intersect1, glm::vec3& intersect2);

};

#endif // !GEOMETERY_HEADER
