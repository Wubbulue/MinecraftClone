#ifndef GEOMETERY_HEADER
#define GEOMETERY_HEADER

#include "glm/glm.hpp"


struct Triangle {
	glm::vec3 points[3];
};

//typedef glm::vec3 Triangle[3];

//Möller–Trumbore intersection algorithm from wikipedia
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


class Box3
{
public:
	Box3(const glm::vec3& vmin, const glm::vec3& vmax)
	{
		bounds[0] = vmin;
		bounds[1] = vmax;
	}
	glm::vec3 bounds[2];

	bool intersect(const Ray& r) const
	{
		float tmin, tmax, tymin, tymax, tzmin, tzmax;

		tmin = (bounds[r.sign[0]].x - r.orig.x) * r.invdir.x;
		tmax = (bounds[1 - r.sign[0]].x - r.orig.x) * r.invdir.x;
		tymin = (bounds[r.sign[1]].y - r.orig.y) * r.invdir.y;
		tymax = (bounds[1 - r.sign[1]].y - r.orig.y) * r.invdir.y;

		if ((tmin > tymax) || (tymin > tmax))
			return false;
		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;

		tzmin = (bounds[r.sign[2]].z - r.orig.z) * r.invdir.z;
		tzmax = (bounds[1 - r.sign[2]].z - r.orig.z) * r.invdir.z;

		if ((tmin > tzmax) || (tzmin > tmax))
			return false;
		if (tzmin > tmin)
			tmin = tzmin;
		if (tzmax < tmax)
			tmax = tzmax;

		return true;
	}

};

#endif // !GEOMETERY_HEADER
