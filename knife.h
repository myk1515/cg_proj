#ifndef KNIFE_H
#define KNIFE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "shader.h"
#include "mesh.h"
#include "cylinder.h"


class Head {
public:
	glm::vec3 h, p1, p2;
	Head(glm::vec3 h, glm::vec3 p1, glm::vec3 p2) {
		this->h = h;
		this->p1 = p1;
		this->p2 = p2;
	}
	Head() {

	}
	void add(glm::vec3 delta) {
		this->h = this->h + delta;
		this->p1 = this->p1 + delta;
		this->p2 = this->p2 + delta;
	}
};


class Knife {
private:
	void setupKnife();
	void loadTexture(unsigned int* texture, std::string path);
	Material material;
	Mesh mesh;
public:
	unsigned int VAO;
	Head head;
	enum Knife_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};
	glm::mat4 shift;
	//knife head reduced to a triangle
	Knife();
	Knife(glm::vec3 h, glm::vec3 p1, glm::vec3 p2, std::string material);
	void move(Knife_Movement direction, float delta);
	void draw(Shader& shader);

	void reset(glm::vec3 head_h);
};


#endif