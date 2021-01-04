#ifndef PARTICLE_H
#define PARTICLE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include "cylinder.h"

class Particle {
public:
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 shift;
	Material material;
	Mesh mesh;
	float age;
	float life;
	void constructMesh();
	Particle();
	Particle(glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, Material material, float age, float life);
	void draw(Shader& shader);
	void reborn(glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration);
};

class ParticleSystem {
private:
	float gravity;
	int availVectorPos;
	int life;
	std::vector<Particle> particles;
public:
	ParticleSystem();
	ParticleSystem(float gravity);
	void init();
	void aging(float dt);
	void kinematics(float dt);
	void draw(Shader& shader);
	void simulate(float dt);
	void addParticles(glm::vec3 position, glm::vec3 velocity, Material material);
};
#endif