#ifndef WHEEL_H
#define WHEEL_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "include/mesh.h"
#include "include/shader.h"


class Wheel {
private:
	glm::mat4 inverseMat;
	glm::vec3 position;
	glm::vec3 center;
	glm::vec3 controlPoint;
	float d;
	float radius;
	unsigned int wheelVAO;
	void setupWheel();
public:
	Wheel();
	Wheel(glm::vec3 center);
	glm::vec3 ProcessMouseMovement(float x, float y);
	void draw(Shader& wheelShader);
	void setInverseMat(glm::mat4 mat);
	void resetControlPoint();
};

#endif