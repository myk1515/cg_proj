#include "particle.h"




Particle::Particle() {

}



Particle::Particle(glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, Material material, float age, float life) {
	this->position = position;
	this->velocity = velocity;
	this->acceleration = acceleration;
	this->age = age;
	this->life = life;
	this->material = material;
	this->shift = glm::vec3(0.0f);
	this->constructMesh();
}

void Particle::constructMesh() {
	float size = 0.01f;
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	Vertex vertex;
	for (int i = -1;i <= 1;i += 2)
		for (int j = -1;j <= 1; j+= 2)
			for (int k = -1; k <= 1; k += 2) {
				vertex.Position = position + glm::vec3(size * i, size * j, size * k);
				vertex.TexCoords = glm::vec3(0.0f);
				vertex.Normal = glm::normalize(glm::vec3((float)i, (float)j, (float)k));
				vertices.push_back(vertex);
			}
	int indices2[36] = {
		0,1,3,0,2,3,
		1,3,7,1,5,7,
		0,1,5,0,4,5,
		5,4,7,5,4,6,
		2,4,6,2,0,6,
		3,2,6,3,2,7,
	};
	for (int i = 0; i < 36; i++) {
		indices.push_back(indices2[i]);
	}

	vector<Texture> textures;
	this->mesh = Mesh(vertices, indices, textures);

}

void Particle::draw(Shader& shader) {
	shader.use();
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shader.ID, "texture_diffuse"), 0);
	glBindTexture(GL_TEXTURE_2D, material.diffuse);

	//glm::mat4 model = glm::translate(glm::mat4(1.0f), shift);
	//shader.setMat4("model", model);
	shader.setVec3("specular", material.specular);
	shader.setFloat("shinness", material.shinness);
	this->mesh.Draw(shader);

}

void Particle::reborn(glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration) {
	this->position = position;
	this->velocity = velocity;
	this->acceleration = acceleration;
	this->age = 0.0f;
	this->shift = glm::vec3(0.0f);
	this->constructMesh();
}





ParticleSystem::ParticleSystem() {

}
ParticleSystem::ParticleSystem(float gravity) {
	availVectorPos = 0;
	this->gravity = gravity;
	this->life = 1.0f;
}
void ParticleSystem::init() {

}
void ParticleSystem::aging(float dt) {
	int i = 0;
	for (std::vector<Particle>::iterator it = particles.begin(); it != particles.end(); it++, i++) {
		if (it->age < it->life) {
			it->age += dt;
			if (it->age > it->life) {
				availVectorPos = std::min(availVectorPos, i);
			}
		}
	}
}
void ParticleSystem::kinematics(float dt) {
	for (std::vector<Particle>::iterator it = particles.begin(); it != particles.end(); it++) 
		if (it->age < it->life){
			it->shift = it->shift + it->velocity * dt;
			it->position = it->position + it->velocity * dt;
			it->velocity = it->velocity + it->acceleration * dt;
	//		std::cout << "acceleration :" << it->acceleration.x << "," << it->acceleration.y << "," << it->acceleration.z << std::endl;
			it->constructMesh();
		}
}
void ParticleSystem::draw(Shader& shader) {
	int i = 0;
	int count = 0;
	//std::cout << "round" << std::endl;
	for (std::vector<Particle>::iterator it = particles.begin(); it != particles.end(); i++,it++) {
		if (it->age < it->life) {
	/*		if (i == 0) {
				std::cout << "id: " << i << std::endl;
				std::cout << "velocity: " << it->velocity.x << "," << it->velocity.y << "," << it->velocity.z << std::endl;
				std::cout << "position: " << it->shift.x << "," << it->shift.y << "," << it->shift.z << std::endl << std::endl;
			}*/
	//		count++;
			it->draw(shader);
		}
	}
//	std::cout << "number of particle: " << count << std::endl;
}

void ParticleSystem::simulate(float dt) {
	aging(dt);
	kinematics(dt);
}

void ParticleSystem::addParticles(glm::vec3 position, glm::vec3 velocity, Material material) {
	int size = particles.size(), i;
	for (int i = availVectorPos; i < size; i++) 
		if (particles[i].age > particles[i].life) {
			particles[i].reborn(position, velocity, glm::vec3(0.0f, gravity, 0.0f));
			return;
		}
	Particle p = Particle(position, velocity, glm::vec3(0.0f, gravity, 0.0f), material, 0.0f, life);
	particles.push_back(p);
}