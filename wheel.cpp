#include "wheel.h"

Wheel::Wheel() {

}



Wheel::Wheel(glm::vec3 center) {
	d = 0.05f;
	radius = 0.05f;
	this->position = center + glm::vec3(0.0f, 0.5f, 0.0f);
	this->center = center;
	setupWheel();
	resetControlPoint();
}

void Wheel::resetControlPoint() {
	controlPoint = center;
}


void Wheel::setupWheel() {
	float x = position.x;
	float y = position.y;
	float z = position.z;
	float vertices[27] = {
		x, y, z,
		x - 2*d, y - 2*d, z,
		x + 2*d, y - 2*d, z,

		x - d, y - 2*d, z,
		x - d, y - 6*d, z,
		x + d, y - 6*d, z,


		x - d, y - 2 * d, z,
		x + d, y - 2 * d, z,
		x + d, y - 6 * d, z,

	};
	for (int i = 0; i < 27; i++)
		std::cout << vertices[i] << " ";

	glGenVertexArrays(1, &wheelVAO);
	unsigned int VBO;
	glBindVertexArray(wheelVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);
}


glm::vec3 Wheel::ProcessMouseMovement(float x, float y) {
	glm::vec4 mousePos = glm::vec4(x, y, 0.0f, 1.0f);
	mousePos = inverseMat * mousePos;
	mousePos = mousePos * 5.0f;
	glm::vec3 curPos = glm::vec3(mousePos.x, mousePos.y, 0.0f);
	//std::cout << mousePos.x << "," << mousePos.y << std::endl;
	//std::cout << controlPoint.x << "," << controlPoint.y << std::endl;
	if (glm::distance(curPos, controlPoint) < radius) {
		controlPoint = curPos;
		return center - curPos;
	}
	return glm::vec3(0.0f);
}

void Wheel::draw(Shader& wheelShader) {

	//std::cout << d << std::endl;
	glm::mat4 model = glm::mat4(1.0f);
	wheelShader.use();
	wheelShader.setVec3("color", glm::vec3(1.0f));

	for (int i = 0; i < 4; i++) {
		model = glm::mat4(1.0f);
		model = glm::translate(model, center);
		model = glm::rotate(model,glm::radians(90.f * i), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, -center);
		wheelShader.setMat4("model",model);
		glBindVertexArray(wheelVAO);
		glDrawArrays(GL_TRIANGLES, 0, 9);
	}

	std::vector<Vertex> vertices;
	Vertex vertex;
	int num = 180;
	for (int i = 0; i < num; i++) {
		float angle = glm::radians((float)i * (360 / num));
		glm::vec3 dir = glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f);
		vertex.Position = controlPoint + dir * radius;
		vertices.push_back(vertex);
	}
	std::vector<unsigned int> indices;
	std::vector<Texture> texture;
	vertex.Position = controlPoint;
	vertices.push_back(vertex);
	for (int i = 0; i < num; i++) {
		int j = (i + 1) % num;
		indices.push_back(i);
		indices.push_back(j);
		indices.push_back(180);
	}
	Mesh mesh = Mesh(vertices, indices, texture);

	wheelShader.setMat4("model", glm::mat4(1.0f));
	mesh.Draw(wheelShader);

}


void Wheel::setInverseMat(glm::mat4 mat) {
	inverseMat = mat;
}