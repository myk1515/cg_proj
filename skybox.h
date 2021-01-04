#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>


#include "include/shader.h"

class Skybox {
private:
	unsigned int skyboxVAO;
	unsigned int cubemapTexture;
	unsigned int loadCubemap(std::vector<std::string> faces);
public:
	Skybox();
	Skybox(int tag);
	void draw(Shader& skyboxShader);
};

