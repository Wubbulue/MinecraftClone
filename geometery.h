#ifndef GEOMETERY_HEADER
#define GEOMETERY_HEADER

#include "Camera.h"
#include <algorithm>


struct Triangle {
	glm::vec3 points[3];
};

struct Plane {
    // unit vector
    glm::vec3 normal = { 0.f, 1.f, 0.f };

    // distance from origin to the nearest point in the plan
    float distance = 0.f;

    Plane() = default;

    Plane(const glm::vec3 & p1, const glm::vec3 & norm)
        : normal(glm::normalize(norm)),
        distance(glm::dot(normal, p1))
    {}

    float getSignedDistanceToPlane(const glm::vec3 & point) const
    {
        return glm::dot(normal, point) - distance;
    }
};

struct Frustum
{
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
};

Frustum createFrustumFromCamera(const Camera& cam, float aspect);

struct Sphere 
{
    glm::vec3 center{ 0.f, 0.f, 0.f };
    float radius{ 0.f };

    Sphere(const glm::vec3& inCenter, float inRadius)
        : center{ inCenter }, radius{ inRadius }
    {}

    bool isOnOrForwardPlane(const Plane& plane) const;

    bool isOnFrustum(const Frustum& camFrustum) const;
};

//this code was stolen from https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling i don't understand it fully UNDERSTAND
struct SquareAABB 
{
    glm::vec3 center{ 0.f, 0.f, 0.f };
    float extent{ 0.f };

    SquareAABB(const glm::vec3& inCenter, float inExtent)
        : center{ inCenter }, extent{ inExtent }
    {}

    bool isOnOrForwardPlane(const Plane& plane) const;

    bool isOnFrustum(const Frustum& camFrustum) const;

};



//Möller–Trumbore intersection algorithm from wikipedia to calculate intersection of ray and triangle
bool rayIntersect(glm::vec3 rayOrigin, glm::vec3 rayVector, Triangle* inTriangle, glm::vec3& outIntersectionPoint);

class Ray
{
public:
    //dir will always be normalized
    Ray(const glm::vec3& orig, glm::vec3& inDir) : orig(orig)
    {
        dir = glm::normalize(inDir);
        invdir = glm::vec3(1.0f,1.0f,1.0f) / dir;
        sign[0] = (invdir.x < 0);
        sign[1] = (invdir.y < 0);
        sign[2] = (invdir.z < 0);
    }
    glm::vec3 orig, dir;       // ray orig and dir 

    //TODO: remove these from ray
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

    //calculate intersection of box and triangle and return result as a T value
    bool intersectParametric(const Ray& r, float& intersect1, float& intersect2);

    //check if point is inside box, inclusive at start and exclusive at end
    bool checkIfInside(glm::vec3 v);

};

#endif // !GEOMETERY_HEADER
