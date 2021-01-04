#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <time.h> 
#include <iostream>
#include <stdlib.h>

#include "include/shader.h"
#include "include/camera.h"
#include "cylinder.h"
#include "knife.h"
#include "bezier.h"
#include "particle.h"
#include "skybox.h"
#include "workpiece.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
     
int window_width,window_height;
bool mouse_press = false;
const float init_radius = 0.5f;
const float init_length = 5.0f;
float auto_mode_t = 0.0f;
std::string material = "metal";

const enum work_mode {
	FREE, DRAW_BEZIER, AUTO_CUT
};
work_mode current_mode = work_mode::FREE;

glm::vec3 lightPos(4.0f, 20.0f, 1.0f);

Camera camera(glm::vec3(0.0f, 3.5f, 5.0f), glm::normalize(glm::vec3(0.0f, 5.0f, -3.5f)));
Camera curveCamera(glm::vec3(0.0f, 0.0f, 5.0f));
Knife knife;
BezierCurve bezierCurve;
Workpiece workpiece;
Skybox skybox;
float lastX = window_width / 2.0f;
float lastY = window_height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;







void drawBezierMode(GLFWwindow* window, Shader& curveShader) {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	processInput(window);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//use
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = curveCamera.GetViewMatrix();
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (float)window_width / window_height, 0.1f, 100.0f);
	//projection = glm::mat4(1.0f);
	curveShader.setMat4("model", model);
	curveShader.setMat4("view", view);
	curveShader.setMat4("projection", projection);
	curveShader.setVec3("color", glm::vec3(0.0f, 0.0f, 0.0f));
	bezierCurve.setInverseMat(glm::inverse(projection * view * model));

	//draw outline
	float axis_vertices[] = {
		-init_length / 2 - 0.5, 0.0f, 0.0f,
		init_length / 2 + 0.5, 0.0f, 0.0f,
		-init_length / 2, 0.0f, 0.0f,
		-init_length / 2, init_radius, 0.0f,
		-init_length / 2, init_radius, 0.0f,
		init_length / 2, init_radius, 0.0f,
		init_length / 2, init_radius, 0.0f,
		init_length / 2, 0, 0.0f,

	};
	unsigned int axis_VAO, axis_VBO;
	glGenVertexArrays(1, &axis_VAO);
	glBindVertexArray(axis_VAO);
	glGenBuffers(1, &axis_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, axis_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_vertices), axis_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	curveShader.use();
	glBindVertexArray(axis_VAO);
	glDrawArrays(GL_LINES, 0, 8);
	bezierCurve.drawPoint(curveShader);
	bezierCurve.drawCurve(curveShader);

	//symmetry
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	curveShader.setMat4("model", model);
	bezierCurve.drawCurve(curveShader);
	glBindVertexArray(axis_VAO);
	glDrawArrays(GL_LINES, 0, 8);
	float vertices[] = {
	   0.5f,  0.5f, 0.0f,  // top right
	   0.5f, -0.5f, 0.0f,  // bottom right
	  -0.5f, -0.5f, 0.0f,  // bottom left
	  -0.5f,  0.5f, 0.0f   // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,  // first Triangle
		1, 2, 3   // second Triangle
	};
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);
	glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void autoStride() {
	float stride = 0.0005;
	glm::vec3 curPos = bezierCurve.sample(auto_mode_t);
	glm::vec3 nextPos = bezierCurve.sample(auto_mode_t + stride);
	auto_mode_t += stride;
	float delta_x = nextPos.x - curPos.x;
	float delta_z = nextPos.y - curPos.y;
	knife.move(Knife::BACKWARD, delta_z);
	knife.move(Knife::RIGHT, delta_x);
}

void cutMode(GLFWwindow* window, Shader& workpieceShader, Shader& particleShader, Shader& testShader, Shader&skyboxShader) {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	processInput(window);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//use
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (current_mode == AUTO_CUT) {
		if (auto_mode_t <= 1.0f) {
			autoStride();
		}
	}
	workpiece.cut(knife.head);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (float)window_width / window_height, 0.1f, 100.0f);
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
	particleShader.use();
	particleShader.setMat4("model", glm::mat4(1.0f));
	particleShader.setMat4("view", view);
	particleShader.setMat4("projection", projection);
	particleShader.setVec3("lightPos", lightPos);
	particleShader.setMat3("normalMat", normalMat);
	particleShader.setVec3("viewPos", camera.Position);

	workpiece.draw(workpieceShader, particleShader);

	testShader.use();
	model = glm::mat4(1.0f);
	model = knife.shift * model;
	testShader.setMat4("model", model);
	testShader.setMat4("projection", projection);
	testShader.setMat4("view", view);
	knife.draw(testShader);


	skyboxShader.use();
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, -20.0f, 5.0f));
	skyboxShader.setMat4("model", model);
	skyboxShader.setMat4("view", view);
	skyboxShader.setMat4("projection", projection);
	skybox.draw(skyboxShader);
}

int main() {
	srand((unsigned)time(NULL));
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	
	GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(pMonitor);
	window_width = mode->width * 0.8;
	window_height = mode->height * 0.8;

	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "CG_project", NULL, NULL);
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
	glViewport(0, 0, window_width, window_height);


	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);



	Shader workpieceShader("./shaders/mainShader.vs", "./shaders/mainShader.fs");
	Shader particleShader("./shaders/mainShader.vs", "./shaders/mainShader.fs");
	Shader testShader("./shaders/test.vs", "./shaders/test.fs");
	Shader curveShader("./shaders/curveShader.vs", "./shaders/curveShader.fs");
	Shader skyboxShader("./shaders/skyboxShader.vs", "./shaders/skyboxShader.fs");
	workpiece = Workpiece(init_length, init_radius, glm::vec3(-init_length / 2, 0.0f, 0.0f), material);
	knife = Knife(glm::vec3(-init_length / 2, 0.0f, init_radius), glm::vec3(-init_length / 2 - 0.05f, 0.0f, init_radius + 0.05f), glm::vec3(-init_length / 2 + 0.05f, 0.0f, init_radius + 0.05f));
	bezierCurve = BezierCurve(glm::vec3(-init_length / 2, 0.0f, 0.0f), glm::vec3(-init_length / 4, 0.0f, 0.0f), glm::vec3(init_length / 4, 0.0f, 0.0f), glm::vec3(init_length / 2, 0.0f, 0.0f));
	skybox = Skybox(0);

	while (!glfwWindowShouldClose(window)) {
		if (current_mode == FREE || current_mode == AUTO_CUT) {
			cutMode(window, workpieceShader, particleShader, testShader, skyboxShader);
		}
		else if (current_mode == DRAW_BEZIER) {
			drawBezierMode(window, curveShader);
		}
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
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		current_mode = FREE;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		current_mode = DRAW_BEZIER;
	}


	if (current_mode == FREE) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);

		float movementSpeed = 0.1f;
		float movement = deltaTime * movementSpeed;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			knife.move(Knife::FORWARD, movement);
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			knife.move(Knife::BACKWARD, movement);
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			knife.move(Knife::LEFT, movement);
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			knife.move(Knife::RIGHT, movement);
	}
	else if (current_mode == DRAW_BEZIER) {
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
			current_mode = AUTO_CUT;
			auto_mode_t = 0.0f;
			glm::vec3 start = bezierCurve.sample(0.0f);
			knife.reset(glm::vec3(start.x, 0, start.y));
		}
	}
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

	if (current_mode == FREE) {
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
	else if (current_mode == DRAW_BEZIER) {
		if (mouse_press) {
			bezierCurve.ProcessMouseMovement(xpos / window_width * 2 - 1.0f, -(ypos / window_height * 2 - 1.0f));
		}
	}


	lastX = xpos;
	lastY = ypos;

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (current_mode == DRAW_BEZIER) {
		if (action == GLFW_PRESS) 
			switch (button) {
				case GLFW_MOUSE_BUTTON_LEFT: {
					mouse_press = true;
					break;
				}
			}
		if (action == GLFW_RELEASE) 
			switch (button) {
				case GLFW_MOUSE_BUTTON_LEFT: {
					mouse_press = false;
					break;
				}
			}
	}
	return;
}