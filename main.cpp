#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "include/shader.h"
#include "include/camera.h"
#include "include/model.h"
#include "include/cylinder.h"
#include "include/knife.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
glm::vec3 lightPos(1.2f, 20.0f, 1.0f);

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
Knife knife(glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 2.0f), glm::vec3(2.0f, 0.0f, 2.0f));
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

#define interval_d 0.01f


class Workpiece {
private:
	float length;
	glm::vec3 leftCenterPos;
	float radius;
	vector<Cylinder> cylinders;

	void loadTexture(unsigned int* texture, std::string path) {
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
public:
	Material sideMaterial, sectionMaterial;
	Workpiece(float length, int radius, glm::vec3 leftCenterPos, std::string material) {
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
			double test = delta / 10;

			cylinders.push_back(Cylinder(radius + test, leftCenterPos + glm::vec3(delta, 0.0f, 0.0f), interval_d, sideMaterial, sectionMaterial));
		}

	}

	void draw(Shader& shader) {
		int size = cylinders.size();
		for (int i = 0; i < size; i++) {
			cylinders[i].draw(shader);
		}
	}

	void cut(Head head) {
		int size = cylinders.size();
		glm::vec3 h = head.h, p1 = head.p1, p2 = head.p2;
		for (int i = 0; i < size; i++) {
			float z_prime;
			bool collision = false;
			float cur_x = leftCenterPos.x + interval_d * i;
			if (cur_x >= p1.x && cur_x <= p2.x) {
				if (h.x != p1.x) {
					z_prime = (h.z - p1.z) / (h.x - p1.x) * (cur_x - h.x) + h.z;
				}
				else {
					z_prime = (h.z - p2.z) / (h.x - p2.x) * (cur_x - h.x) + h.z;
				}
				if (z_prime < cylinders[i].radius) {
					cylinders[i].uncut = false;
					cylinders[i].setRadius(z_prime);
					//		cout << "z_prime: " << z_prime << endl;
				}
			}
		}
	}

};


int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CG_project", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//viewport dimension left-bottom right-top
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);


	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);



	Shader workpieceShader("./shaders/vs.shader", "./shaders/fs.shader");
	Shader testShader("./shaders/test.vs", "./shaders/test.fs");
	Workpiece workpiece = Workpiece(3.0f, 0.2f, glm::vec3(-1.5f, 0.0f, 0.0f), "wood");

	while (!glfwWindowShouldClose(window)) {

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//use
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		workpiece.cut(knife.head);

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, (float)glfwGetTime() * 50, glm::vec3(1.0f, 0.0f, 0.0f));
		workpieceShader.use();
		workpieceShader.setMat4("model", model);
		glm::mat4 normalMat = glm::mat3(glm::transpose(glm::inverse(model)));
		workpieceShader.setMat4("view", view);
		workpieceShader.setMat4("projection", projection);
		workpieceShader.setVec3("lightPos", lightPos);
		workpieceShader.setMat3("normalMat", normalMat);
		workpieceShader.setVec3("viewPos", camera.Position);


		workpiece.draw(workpieceShader);

		unsigned int VAO;

		float vertices[] = {
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 2.0f, 0.0f, 1.0f, 0.0f,
			2.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f,
		};
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		testShader.use();
		model = glm::mat4(1.0f);
		model = knife.shift * model;
		testShader.setMat4("model", model);
		testShader.setMat4("projection", projection);
		testShader.setMat4("view", view);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;

}



void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		knife.ProcessKeyboard(Knife::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		knife.ProcessKeyboard(Knife::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		knife.ProcessKeyboard(Knife::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		knife.ProcessKeyboard(Knife::RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}