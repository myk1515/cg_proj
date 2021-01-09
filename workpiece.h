#ifndef WORKPIECE_H
#define WORKPIECE_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <time.h> 
#include <iostream>
#include <stdlib.h>

#include "include/shader.h"
#include "cylinder.h"
#include "particle.h"
#include "knife.h"

#define interval_d 0.01f


class Workpiece {
private:
	float length;
	glm::vec3 leftCenterPos;
	float radius;
	vector<Cylinder> cylinders;
	vector<bool> isChanged;
	ParticleSystem ps;

	void loadTexture(unsigned int* texture, std::string path);
	void generateParticle(glm::vec3 position);
	void adjustCylinder(int i);
public:
	Material sideMaterial, sectionMaterial;
	Workpiece();
	Workpiece(float length, float radius, glm::vec3 leftCenterPos, std::string material);
	void draw(Shader& workpieceShader, Shader& particleShader);

	void cut(Head head);

	void drawOutline(Shader& curveShader);
};


#endif