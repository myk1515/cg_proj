#include "bezier.h"
#include "mesh.h"
#include <iostream>

BezierCurve::BezierCurve(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) {
	controlPoints.push_back(a);
	controlPoints.push_back(b);
	controlPoints.push_back(c);
	controlPoints.push_back(d);
	inverseMat = glm::mat4(1.0f);
	//glGenVertexArrays(1, &VAO);
}

BezierCurve::BezierCurve() {

}

void BezierCurve::drawPoint(Shader& shader) {
	float delta = 0.03f;
	int size = controlPoints.size();
	vector<unsigned int> indices;
	indices.push_back(0); indices.push_back(1); indices.push_back(2); indices.push_back(2); indices.push_back(3); indices.push_back(0);
	for (int i = 0; i < size; i++) {
		glm::vec3 point = controlPoints[i];
		vector<Vertex> vertices;
		Vertex vertex;
		vertex.Position = point + glm::vec3(-delta, delta, 0);    
		vertices.push_back(vertex);
		vertex.Position = point + glm::vec3(delta, delta, 0);
		vertices.push_back(vertex);
		vertex.Position = point + glm::vec3(delta, -delta, 0);
		vertices.push_back(vertex);
		vertex.Position = point + glm::vec3(-delta, -delta, 0);
		vertices.push_back(vertex);
		vector<Texture> textures;
		Mesh cur = Mesh(vertices, indices, textures);
		cur.Draw(shader);
	}
}

void BezierCurve::drawCurve(Shader& shader) {

	float delta = 0.01f;
	int arrLength = 0;
	glm::vec3 point = sample(0.0f);
	vertices[arrLength++] = point.x; vertices[arrLength++] = point.y; vertices[arrLength++] = point.z;
	for (float t = delta; t < 1.0f; t += delta) {
		point = sample(t);
		vertices[arrLength++] = point.x; vertices[arrLength++] = point.y; vertices[arrLength++] = point.z;
		vertices[arrLength++] = point.x; vertices[arrLength++] = point.y; vertices[arrLength++] = point.z;
	}
	point = sample(1.0f);
	vertices[arrLength++] = point.x; vertices[arrLength++] = point.y; vertices[arrLength++] = point.z;

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	unsigned int VBO;
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	shader.use();
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, arrLength / 3);

}

void BezierCurve::ProcessMouseMovement(float x, float y) {
	glm::vec4 mousePos = glm::vec4(x, y, 0.0f, 1.0f);
	mousePos = inverseMat * mousePos;
	mousePos = mousePos * 5.0f;
	glm::vec3 curPos = glm::vec3(mousePos.x, mousePos.y, 0.0f);
	int size = controlPoints.size();
	for (int i = 0; i < size; i++) {
		glm::vec3 pos = controlPoints[i];
		float distance = glm::dot(curPos - pos, curPos - pos);

	//	std::cout << "mousepos " << i << ":" << curPos.x << "," << curPos.y << "," << curPos.z << std::endl;
	//	std::cout << "pointpos:" << pos.x << "," << pos.y << "," << pos.z << std::endl;
	//	std::cout << "distance: " << distance << std::endl;
		if (glm::dot(curPos - pos, curPos - pos) < 1e-2)
			controlPoints[i] = curPos;
	}
}

void BezierCurve::setInverseMat(glm::mat4 inverseMat) {
	this->inverseMat = inverseMat;
}

glm::vec3 BezierCurve::sample(float t) {
	std::vector<glm::vec3> points = controlPoints;
	while (points.size() != 1) {
		std::vector<glm::vec3> temp;
		for (int i = 0; i < points.size() - 1; i++) {
			temp.push_back((1.0f - t) * points[i] + t * points[i + 1]);
		}
		points = temp;
	}
	return points[0];
}


float* BezierCurve::sampleAll(float start) {
	for (int i = 0; i < 1000; i++)
		y_x[i] = -1.0f;
	float interval_d = 0.01f;
	float xpos = start;
	int i = 0;
	for (float t = 0.0f; t < 1.0f; t += 0.0001f) {
		glm::vec3 p = sample(t);
		if (p.x > start && std::fabs(xpos - p.x) < 1e-2) {
			if (p.y > 0.5f)
				y_x[i] = 0.5f;
			else
				y_x[i] = p.y;
			i++; xpos += interval_d;
		}
	}
	for (int i = 0; i < 500; i++)
		std::cout << y_x[i] << " ";
	std::cout << std::endl;

	return y_x;

}