
#include "knife.h"
#include "include/stb_image.h"



void Knife::setupKnife() {
	float dz = 0.5f;
	float dy = 0.2f;
	float dx = (head.p2.x - head.p1.x);
	glm::vec3 center = glm::vec3((head.p2.x + head.p1.x) / 2, 0.0f, dz + head.p2.z);
	float x = head.p1.x, y = head.p1.y + dy / 2, z = head.p1.z;
	float vertices2[180] = {
		x, y, z, 0.0f, 1.0f, 0.0f,
		x + dx, y, z, 0.0f, 1.0f, 0.0f,
		x + dx, y, z + dz, 0.0f, 1.0f, 0.0f,
		x, y, z + dz, 0.0f, 1.0f, 0.0f,

		x + dx, y, z, 1.0f, 0.0f, 0.0f,
		x + dx, y - dy, z, 1.0f, 0.0f, 0.0f,
		x + dx, y - dy, z + dz, 1.0f, 0.0f, 0.0f,
		x + dx, y, z + dz, 1.0f, 0.0f, 0.0f,

		x, y, z + dz, 0.0f, 0.0f, 1.0f,
		x, y - dy, z + dz, 0.0f, 0.0f, 1.0f,
		x + dx, y - dy, z + dz, 0.0f, 0.0f, 1.0f,
		x + dx, y, z + dz, 0.0f, 0.0f, 1.0f,

		x, y, z, 0.0f, 0.0f, -1.0f,
		x, y - dy, z, 0.0f, 0.0f, -1.0f,
		x + dx, y - dy, z, 0.0f, 0.0f, -1.0f,
		x + dx, y, z, 0.0f, 0.0f, -1.0f,

		x, y, z, -1.0f, 0.0f, 0.0f,
		x, y - dy, z, -1.0f, 0.0f, 0.0f,
		x, y - dy, z + dz, -1.0f, 0.0f, 0.0f,
		x, y, z + dz, -1.0f, 0.0f, 0.0f,

		x, y - dy, z, 0.0f, -1.0f, 0.0f,
		x + dx, y - dy, z, 0.0f, -1.0f, 0.0f,
		x + dx, y - dy, z + dz, 0.0f, -1.0f, 0.0f,
		x, y - dy, z + dz, 0.0f, -1.0f, 0.0f,

		x, y, z,
		x + dx, y, z,
		head.h.x, head.h.y, head.h.z,

		x + dx, y, z,
		x + dx, y - dy, z,
		head.h.x, head.h.y, head.h.z,

		x, y, z,
		x, y - dy, z,
		head.h.x, head.h.y, head.h.z,

		x, y - dy, z,
		x + dx, y - dy, z,
		head.h.x, head.h.y, head.h.z,
	};
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	Vertex vertex;
	for (int i = 0; i < 144; i += 6) {
		vertex.Position.x = vertices2[i];
		vertex.Position.y = vertices2[i + 1];
		vertex.Position.z = vertices2[i + 2];
		vertex.Normal.x = vertices2[i + 3];
		vertex.Normal.y = vertices2[i + 4];
		vertex.Normal.z = vertices2[i + 5];
		vertex.TexCoords = glm::vec3(0.0f, 0.0f, 0.0f);
		vertices.push_back(vertex);
	}
	for (int i = 144; i < 180; i += 9) {
		Vertex vertex1, vertex2, vertex3;
		vertex1.Position = glm::vec3(vertices2[i], vertices2[i + 1], vertices2[i + 2]);
		vertex2.Position = glm::vec3(vertices2[i + 3], vertices2[i + 4], vertices2[i + 5]);
		vertex3.Position = glm::vec3(vertices2[i + 6], vertices2[i + 7], vertices2[i + 8]);
		glm::vec3 normal = glm::normalize(glm::cross(vertex1.Position - vertex3.Position, vertex2.Position - vertex3.Position));
		vertex1.Normal = vertex2.Normal = vertex3.Normal = normal;
		vertices.push_back(vertex1);
		vertices.push_back(vertex2);
		vertices.push_back(vertex3);
	}
	for (int i = 0; i < 24; i += 4) {
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);

		indices.push_back(i);
		indices.push_back(i + 3);
		indices.push_back(i + 2);
	}

	for (int i = 24; i < 36; i++)
		indices.push_back(i);
	vector<Texture> textures;
	this->mesh = Mesh(vertices, indices, textures);
}
void Knife::loadTexture(unsigned int* texture, std::string path) {
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

Knife::Knife() {

}
Knife::Knife(glm::vec3 h, glm::vec3 p1, glm::vec3 p2, std::string material) {
	head.h = h;
	head.p1 = p1;
	head.p2 = p2;
	shift = glm::mat4(1.0f);
	loadTexture(&this->material.diffuse, "./material/" + material + "Section.jpg");
	if (material == "metal") {
		this->material.specular = this->material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
		this->material.shinness = this->material.shinness = 32.0f;
	}
	else {
		this->material.specular = this->material.specular = glm::vec3(0.0f, 0.0f, 0.0f);
		this->material.shinness = this->material.shinness = 2.0f;
	}


	setupKnife();

}
void Knife::move(Knife_Movement direction, float delta)
{
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
void Knife::draw(Shader& shader) {
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

void Knife::reset(glm::vec3 head_h) {
	float delta_x = head_h.x - head.h.x;
	float delta_z = head_h.z - head.h.z;
	move(BACKWARD, delta_z);
	move(RIGHT, delta_x);
}