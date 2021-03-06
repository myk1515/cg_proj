#ifndef BEZIER_H
#define BEZIER_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>

#include "include/shader.h"

class BezierCurve {
private:
	std::vector<glm::vec3> controlPoints;
	glm::mat4 inverseMat;
	//unsigned int VAO;
	float vertices[1000];
	float y_x[1000];
public:
	BezierCurve(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d);
	BezierCurve();
	void drawPoint(Shader& shader);
	void drawCurve(Shader& shader);
	void ProcessMouseMovement(float x, float y);
	void moveAll(float x, float y, float lastX, float lastY);
	void setInverseMat(glm::mat4 inverseMat);
	glm::vec3 sample(float t);
	float* sampleAll(float start);
};

#endif