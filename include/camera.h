#ifndef CAMERA_H_
#define CAMERA_H_

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "ray.h"
using namespace glm;

class camera
{
public:
	camera(const vec3& e, const vec3& c, const vec3& u, double focal, double width, double height) :
		eye(e), center(c), up(u), viewToWorld(inverse(lookAt(eye, center, up))),
		focalLength(focal), screenWidth(width), screenHeight(height),
		lowerLeftCornerLocal(getLLCL())	{}
	void setEye(const vec3&);
	void setCenter(const vec3&);
	ray getRayFromScreenPos(double u, double v);
private:
	vec3 getLLCL();
	void updateCamera();
	vec3 eye;
	vec3 center;
	vec3 up;
	mat4 viewToWorld;
	double focalLength;
	double screenWidth;
	double screenHeight;
	vec3 lowerLeftCornerLocal;
};

inline vec3 camera::getLLCL()
{
	return vec3(-screenWidth / 2, .0f, .0f) + vec3(.0f, -screenHeight / 2, .0f) + vec3(.0f, .0f, -focalLength);
}


inline void camera::setEye(const vec3& e)
{
	eye = e;
	updateCamera();
}


inline void camera::setCenter(const vec3& c)
{
	center = c;
	updateCamera();
}

inline void camera::updateCamera()
{
	viewToWorld = inverse(lookAt(eye, center, up));
	lowerLeftCornerLocal = getLLCL();
}

inline ray camera::getRayFromScreenPos(double u, double v)
{
	auto pixelPosLocal = lowerLeftCornerLocal + vec3(0.f, u * screenHeight, 0.f) + vec3(v * screenWidth, 0.f, 0.f);
	return ray(eye, vec3(viewToWorld * vec4(pixelPosLocal, 1.0f))-eye);
}

#endif