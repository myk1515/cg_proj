#ifndef CYLINDER_H
#define CYLINDER_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include "mesh.h"
#include "shader.h"

struct Material {
	unsigned int diffuse;
	glm::vec3 specular;
	float shinness;
};



class Cylinder{
private:
	Mesh sideMesh;
	Mesh sectionMesh;
	float radius;
	glm::vec3 leftCenterPos;
	float d;
	unsigned int sideTexture;
	Material sideMaterial,sectionMaterial;
public:
	Cylinder(float radius, glm::vec3 leftCenterPos, float d, Material sideMaterial, Material sectionMaterial) {
		this->radius = radius;
		this->leftCenterPos = leftCenterPos;
		this->d = d;
		this->sideMaterial = sideMaterial;
		this->sectionMaterial = sectionMaterial;
		setRadius(radius);

	}

	//generate the vertices
	void setRadius(float radius) {
		int num = 360;
		this->radius = radius;
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		for (int i = 0; i < num; i++) {
			Vertex vertex1, vertex2;
			float angle = glm::radians((float)i);
			glm::vec3 dir = glm::vec3(0.0f, glm::cos(angle), glm::sin(angle));
			vertex1.Position = leftCenterPos + dir * radius;
			vertex1.Normal = glm::normalize(dir);
			vertex1.TexCoords = glm::vec2(leftCenterPos.x, (float)angle * radius);
		//	cout << angle << endl;
			vertex2.Position = vertex1.Position + glm::vec3(d, 0.0f, 0.0f);
			vertex2.Normal = glm::normalize(dir);
			vertex2.TexCoords = glm::vec2(leftCenterPos.x + d, (float)angle * radius);
			vertices.push_back(vertex1);
			vertices.push_back(vertex2);

		}
		
		for (int i = 0; i < 2 * num; i++) {
			indices.push_back(i);
			indices.push_back((i + 1) % (2 * num));
			indices.push_back((i + 2) % (2 * num));
		}

		vector<Texture> textures;
		this->sideMesh = Mesh(vertices, indices, textures);

		for (int i = 0; i < num; i++) {
			vertices[i * 2].Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
			vertices[i * 2 + 1].Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		}

		Vertex leftCenter, rightCenter;
		leftCenter.Position = leftCenterPos;
		leftCenter.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		leftCenter.TexCoords = leftCenterPos;
		rightCenter.Position = leftCenterPos + glm::vec3(d, 0.0f, 0.0f);
		rightCenter.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		leftCenter.TexCoords = leftCenterPos;
		vertices.push_back(leftCenter); vertices.push_back(rightCenter);
		indices.clear();

		for (int i = 0; i < num; i ++) {
			indices.push_back(i * 2);
			indices.push_back(((i + 1) % num) * 2);
			indices.push_back(2 * num);
			//cout << vertices[i * 2].Position.x << vertices[i * 2].Position.y << vertices[i * 2].Position.z << endl;
			//cout << vertices[i * 2 + 2].Position.x << vertices[i * 2 + 2].Position.y << vertices[i * 2 + 2].Position.z << endl;
			//cout << vertices[num].Posi

			indices.push_back(i * 2 + 1);
			indices.push_back(((i + 1) % num) * 2 + 1);
			indices.push_back(2 * num + 1);
		}
		this->sectionMesh = Mesh(vertices, indices, textures);

	}

	void draw(Shader &shader) {

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(shader.ID, "texture_diffuse"), 0);
		glBindTexture(GL_TEXTURE_2D, sideMaterial.diffuse);

		shader.setVec3("specular", sideMaterial.specular);
		shader.setFloat("shinness", sideMaterial.shinness);
		this->sideMesh.Draw(shader);



		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(shader.ID, "texture_diffuse"), 0);
		glBindTexture(GL_TEXTURE_2D, sectionMaterial.diffuse);


		shader.setVec3("specular", sectionMaterial.specular);
		shader.setFloat("shinness", sectionMaterial.shinness);
		this->sectionMesh.Draw(shader);
	}
};

#endif