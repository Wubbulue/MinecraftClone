#include "geometery.h"

//NOTE: A decent chunk of this code was copied from somewhere, I don't remember where though...

bool rayIntersect(glm::vec3 rayOrigin, glm::vec3 rayVector, Triangle* inTriangle, glm::vec3& outIntersectionPoint) {
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
	u = f * glm::dot(s, h);
	if (u < 0.0 || u > 1.0)
		return false;
	//q = s.crossProduct(edge1);
	q = glm::cross(s, edge1);
	//v = f * rayVector.dotProduct(q);
	v = f * glm::dot(rayVector, q);
	if (v < 0.0 || u + v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.
	//float t = f * edge2.dotProduct(q);
	float t = f * glm::dot(edge2, q);
	if (t > EPSILON) // ray intersection
	{
		outIntersectionPoint = rayOrigin + rayVector * t;
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;

}


bool Box3::intersect(const Ray& r, glm::vec3& intersect1, glm::vec3& intersect2)
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


	intersect1 = tmin * r.dir + r.orig;
	intersect2 = tmax * r.dir + r.orig;

	//TODO: find better rounding solution
	//number of 0s is decimals of precision
	const int precision = 10000;


	intersect1.x = std::round(intersect1.x * precision) / precision;
	intersect1.y = std::round(intersect1.y * precision) / precision;
	intersect1.z = std::round(intersect1.z * precision) / precision;

	intersect2.x = std::round(intersect2.x * precision) / precision;
	intersect2.y = std::round(intersect2.y * precision) / precision;
	intersect2.z = std::round(intersect2.z * precision) / precision;

	return true;
}


bool Box3::intersectParametric(const Ray& r, float& intersect1, float& intersect2) {
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

	intersect1 = tmin;
	intersect2 = tmax;


	return true;

}


bool Box3::checkIfInside(glm::vec3 v) {

	bool xBound = (v.x >= bounds[0].x) && (v.x < bounds[1].x);

	bool yBound = (v.y >= bounds[0].y) && (v.y < bounds[1].y);

	bool zBound = (v.z >= bounds[0].y) && (v.z < bounds[1].z);

	return xBound && yBound && zBound;

}

Frustum createFrustumFromCamera(const Camera& cam, float aspect)
{
	Frustum     frustum;
	const float halfVSide = cam.far * tanf(glm::radians(cam.Zoom * .5f));
	const float halfHSide = halfVSide * aspect;
	const glm::vec3 frontMultFar = cam.far * cam.direction;

	frustum.nearFace = { cam.position + cam.near * cam.direction, cam.direction };
	frustum.farFace = { cam.position + frontMultFar, -cam.direction };
	frustum.rightFace = { cam.position, glm::cross(cam.up, frontMultFar + cam.right * halfHSide) };
	frustum.leftFace = { cam.position, glm::cross(frontMultFar - cam.right * halfHSide, cam.up) };
	frustum.topFace = { cam.position, glm::cross(cam.right, frontMultFar - cam.up * halfVSide) };
	frustum.bottomFace = { cam.position, glm::cross(frontMultFar + cam.up * halfVSide, cam.right) };

	return frustum;
}

bool Sphere::isOnOrForwardPlane(const Plane& plane) const
{
	return plane.getSignedDistanceToPlane(center) > -radius;
}

bool Sphere::isOnFrustum(const Frustum& camFrustum) const
{

	//Check Firstly the result that have the most chance to faillure to avoid to call all functions.
	return (isOnOrForwardPlane(camFrustum.leftFace) &&
		isOnOrForwardPlane(camFrustum.rightFace) &&
		isOnOrForwardPlane(camFrustum.farFace) &&
		isOnOrForwardPlane(camFrustum.nearFace) &&
		isOnOrForwardPlane(camFrustum.topFace) &&
		isOnOrForwardPlane(camFrustum.bottomFace));
}

bool SquareAABB::isOnOrForwardPlane(const Plane& plane) const
{
	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	const float r = extent * (std::abs(plane.normal.x) + std::abs(plane.normal.y) + std::abs(plane.normal.z));
	return -r <= plane.getSignedDistanceToPlane(center);
}

bool SquareAABB::isOnFrustum(const Frustum& camFrustum) const
{

	return (isOnOrForwardPlane(camFrustum.leftFace) &&
		isOnOrForwardPlane(camFrustum.rightFace) &&
		isOnOrForwardPlane(camFrustum.topFace) &&
		isOnOrForwardPlane(camFrustum.bottomFace) &&
		isOnOrForwardPlane(camFrustum.nearFace) &&
		isOnOrForwardPlane(camFrustum.farFace));
}
