#include "workpiece.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"


#define FloatPrecision(X,n) ((float)((int)(((X)+(((X)>=0)?5*pow(10,-(n+1)):-5*pow(10,-(n+1))))*pow(10,(n)))/pow(10,(n))))

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
		isChanged.push_back(false);
		cylinders.push_back(Cylinder(radius, radius, radius, leftCenterPos + glm::vec3(delta, 0.0f, 0.0f), interval_d, sideMaterial, sectionMaterial));
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
void Workpiece::adjustCylinder(int id) {
	float left = (id > 0) ? cylinders[id - 1].radius : cylinders[id].radius;
	float right = (id + 1 < cylinders.size()) ? cylinders[id + 1].radius : cylinders[id].radius;
	float r = cylinders[id].radius;
	cylinders[id].setRadius(-1, (left + r) / 2, (right + r) / 2);
}

void Workpiece::cut(Head head) {
	int size = cylinders.size();
	bool cut = false;
	glm::vec3 h = head.h, p1 = head.p1, p2 = head.p2;
	//std::cout << "workpiece :" << h.x << "," << h.y << "," << h.z << std::endl;
	for (int i = 0; i < size; i++) {
		float z_prime;
		bool collision = false;
		float cur_x = leftCenterPos.x + interval_d * i;

		if (cur_x <= h.x && h.x <= cur_x + interval_d) {
			z_prime = h.z;
		}
		else if (p1.x < cur_x && cur_x < h.x) {
			z_prime = (h.z - p1.z) / (h.x - p1.x) * (cur_x - h.x) + h.z;
		}
		else if (h.x < cur_x && cur_x < p2.x) {
			z_prime = (h.z - p2.z) / (h.x - p2.x) * (cur_x - h.x) + h.z;
		}
		else if (cur_x == h.x) {
			z_prime = h.z;
		}
		else
			continue;
		if (z_prime < cylinders[i].radius) {
			if (z_prime < 0) 
				z_prime = 0;
			if (cylinders[i].radius > 0) {
				cut = true;
			}
			cylinders[i].uncut = false;
			if (z_prime != cylinders[i].radius) {
				if (i > 0)
					isChanged[i - 1] = true;
				isChanged[i] = true;
				if (i + 1 < size)
					isChanged[i + 1] = true;
			}
			if (z_prime <= 0)
				cylinders[i].setRadius(0.0f, 0.0f, 0.0f);
			else {
				cylinders[i].setRadius(z_prime, -1.0f, -1.0f);
			}
			//		cout << "z_prime: " << z_prime << endl;
		}
	}

	for (int i = 0; i < size; i++) {
		if (isChanged[i]) {
			adjustCylinder(i);
			isChanged[i] = false;
	//		std::cout << cylinders[i].radius << " " << std::endl;
		}
	}


	if (cut) {
		generateParticle(glm::vec3(head.h.x, head.h.y, head.h.z));
	}
}


void Workpiece::drawOutline(Shader& curveShader) {
	vector <glm::vec3> vertices;

	int size = cylinders.size();
	vertices.push_back(leftCenterPos);
	float x_coords = leftCenterPos.x;
	for (int i = 0; i < size; i++) {
		glm::vec3 v1 = glm::vec3(x_coords, cylinders[i].leftRadius, 0.0f);
		glm::vec3 v2 = glm::vec3(x_coords + interval_d, cylinders[i].rightRadius, 0.0f);
		vertices.push_back(v1); vertices.push_back(v1);
		vertices.push_back(v2); vertices.push_back(v2);
		x_coords += interval_d;
	}
	vertices.push_back(leftCenterPos + glm::vec3(length, 0.0f, 0.0f));

	unsigned int outlineVAO, outlineVBO;
	glGenVertexArrays(1, &outlineVAO);
	glBindVertexArray(outlineVAO);
	glGenBuffers(1, &outlineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	curveShader.use();
	glBindVertexArray(outlineVAO);
	glDrawArrays(GL_LINES, 0, vertices.size());
}