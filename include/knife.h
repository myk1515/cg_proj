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

#include "shader.h"


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
	unsigned int VAO;
	void setupTest() {
		//test
		
	}
public:
	Head head;
	enum Knife_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};
	glm::mat4 shift;
	//knife head reduced to a triangle
	Knife() {

	}
	Knife(glm::vec3 h, glm::vec3 p1, glm::vec3 p2) {
		head.h = h;
		head.p1 = p1;
		head.p2 = p2;
		shift = glm::mat4(1.0f);
		setupTest();

	}
	void ProcessKeyboard(Knife_Movement direction, float deltaTime)
	{
		float MovementSpeed = 0.1f;
		float delta = MovementSpeed * deltaTime;
		if (direction == FORWARD) {
			head.add(glm::vec3(0.0f, 0.0f, -delta));
			shift = glm::translate(shift, glm::vec3(0.0f, 0.0f, -delta));
		}
		if (direction == BACKWARD) {
			head.add(glm::vec3(0.0f, 0.0f, delta));
			shift = glm::translate(shift, glm::vec3(0.0f, 0.0f, delta));
		}
		if (direction == LEFT) {
			head.add(glm::vec3(-delta, 0.0f, 0.0f));
			shift = glm::translate(shift, glm::vec3(-delta, 0.0f, 0.0f));
		}
		if (direction == RIGHT) {
			head.add(glm::vec3(delta, 0.0f, 0.0f));
			shift = glm::translate(shift, glm::vec3(delta, 0.0f, 0.0f));
		}

	/*	cout << "head:" << endl;
		cout << head.h.x << "," << head.h.y << "," << head.h.z << endl;
		cout << head.p1.x << "," << head.p1.y << "," << head.p1.z << endl;
		cout << head.p2.x << "," << head.p2.y << "," << head.p2.z << endl;*/
	}
	void draw(Shader& shader) {
	}
};


#endif