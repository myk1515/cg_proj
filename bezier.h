#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "include/shader.h"

class BezierCurve {
private:
	std::vector<glm::vec3> controlPoints;
	glm::mat4 inverseMat;
	//unsigned int VAO;
	float vertices[1000];
public:
	BezierCurve(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d);
	void drawPoint(Shader& shader);
	void drawCurve(Shader& shader);
	void ProcessMouseMovement(float x, float y);
	void setInverseMat(glm::mat4 inverseMat);
	glm::vec3 sample(float t);
};