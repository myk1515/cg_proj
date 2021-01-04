#include "workpiece.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"



void Workpiece::loadTexture(unsigned int* texture, std::string path) {
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}
void Workpiece::generateParticle(glm::vec3 position) {
	float delta1 = ((float)rand() / 32600 - 0.5f) * 2 * 0.1f;
	float delta2 = ((float)rand() / 32600) * 0.4f + 0.8f;
	float delta3 = ((float)rand() / 32600) * 0.4f + 0.8f;
	for (int i = 0; i < 1; i++) {
		ps.addParticles(position, glm::vec3(2.0f * delta1, 3.0f * delta2, 4.0f * delta3), sectionMaterial);
	}
}



Workpiece::Workpiece() {

}
Workpiece::Workpiece(float length, float radius, glm::vec3 leftCenterPos, std::string material) {
	this->ps = ParticleSystem(-10.0f);
	this->length = length;
	this->radius = radius;
	this->leftCenterPos = leftCenterPos;

	loadTexture(&sideMaterial.diffuse, "./material/" + material + "Side.jpg");
	loadTexture(&sectionMaterial.diffuse, "./material/" + material + "Section.jpg");
	if (material == "metal") {
		sideMaterial.specular = sectionMaterial.specular = glm::vec3(1.0f, 1.0f, 1.0f);
		sideMaterial.shinness = sectionMaterial.shinness = 32.0f;
	}
	else {
		sideMaterial.specular = sectionMaterial.specular = glm::vec3(0.0f, 0.0f, 0.0f);
		sideMaterial.shinness = sectionMaterial.shinness = 2.0f;
	}

	for (float delta = 0; delta < length; delta += interval_d) {
		cylinders.push_back(Cylinder(radius, leftCenterPos + glm::vec3(delta, 0.0f, 0.0f), interval_d, sideMaterial, sectionMaterial));
	}

}

void Workpiece::draw(Shader& workpieceShader, Shader& particleShader) {
	float dt = 0.01f;
	particleShader.use();
	ps.simulate(dt);
	ps.draw(particleShader);
	int size = cylinders.size();
	workpieceShader.use();
	for (int i = 0; i < size; i++) {
		cylinders[i].draw(workpieceShader);
	}
}

void Workpiece::cut(Head head) {
	int size = cylinders.size();
	bool cut = false;
	glm::vec3 h = head.h, p1 = head.p1, p2 = head.p2;
	for (int i = 0; i < size; i++) {
		float z_prime;
		bool collision = false;
		float cur_x = leftCenterPos.x + interval_d * i;
		if (p1.x < cur_x && cur_x < h.x) {
			z_prime = (h.z - p1.z) / (h.x - p1.x) * (cur_x - h.x) + h.z;
		}
		else if (h.x < cur_x && cur_x < p2.x) {
			z_prime = (h.z - p2.z) / (h.x - p2.x) * (cur_x - h.x) + h.z;
		}
		else
			continue;
		if (z_prime < cylinders[i].radius) {
			if (z_prime < 0) z_prime = 0;
			cut = true;
			cylinders[i].uncut = false;
			cylinders[i].setRadius(z_prime);
			//		cout << "z_prime: " << z_prime << endl;
		}
	}

	if (cut) {
		generateParticle(glm::vec3(head.h.x, head.h.y, head.h.z));
	}
}
