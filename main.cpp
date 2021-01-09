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
#include "wheel.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
     
int window_width,window_height;
bool mouse_press = false;
bool alt_press = false;
const float init_radius = 0.5f;
const float init_length = 5.0f;
float auto_mode_t = 0.0f;
std::string material = "wood";
bool stopRotate = false;
bool stopAuto = false;
bool displayControlPoint = true;

const enum work_mode {
	FREE, DRAW_BEZIER, AUTO, CONSTRAIN
};
work_mode current_mode = work_mode::FREE;

glm::vec3 lightPos(0.0f, 20.0f, 1.0f);

Camera camera(glm::vec3(0.0f, 3.5f, 5.0f), glm::normalize(glm::vec3(0.0f, 5.0f, -3.5f)));
Camera curveCamera(glm::vec3(0.0f, 0.0f, 5.0f));
Knife knife;
BezierCurve bezierCurve;
Workpiece workpiece;
Skybox skybox;
Wheel wheel;
glm::vec3 dir = glm::vec3(0.0f);
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
	/*float axis_vertices[] = {
		-init_length / 2 - 0.5, 0.0f, 0.0f,
		init_length / 2 + 0.5, 0.0f, 0.0f,
		-init_length / 2, 0.0f, 0.0f,
		-init_length / 2, init_radius, 0.0f,
		-init_length / 2, init_radius, 0.0f,
		init_length / 2, init_radius, 0.0f,
		init_length / 2, init_radius, 0.0f,
		init_length / 2, 0, 0.0f,

	};*/
	vector <glm::vec3> axis_vertices;
	axis_vertices.push_back(glm::vec3(-init_length / 2 - 0.5, 0.0f, 0.0f));
	axis_vertices.push_back(glm::vec3(init_length / 2 + 0.5, 0.0f, 0.0f));




	unsigned int axis_VAO, axis_VBO;
	glGenVertexArrays(1, &axis_VAO);
	glBindVertexArray(axis_VAO);
	glGenBuffers(1, &axis_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, axis_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * axis_vertices.size(), &axis_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	curveShader.use();
	glBindVertexArray(axis_VAO);
	glDrawArrays(GL_LINES, 0, 2);
	if (!alt_press)
		bezierCurve.drawPoint(curveShader);
	bezierCurve.drawCurve(curveShader);
	workpiece.drawOutline(curveShader);

	//symmetry
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	curveShader.setMat4("model", model);
	bezierCurve.drawCurve(curveShader);
	glBindVertexArray(axis_VAO);
	glDrawArrays(GL_LINES, 0, 2);
	workpiece.drawOutline(curveShader);
	
}

void autoStride() {
	float stride = 0.0001f;
	glm::vec3 curPos = bezierCurve.sample(auto_mode_t);
	glm::vec3 nextPos = bezierCurve.sample(auto_mode_t + stride);
	auto_mode_t += stride;
	float delta_x = nextPos.x - curPos.x;
	float delta_z = nextPos.y - curPos.y;
	knife.move(Knife::BACKWARD, delta_z);
	knife.move(Knife::RIGHT, delta_x);
}

void cutMode(GLFWwindow* window, Shader& workpieceShader, Shader& particleShader, Shader& knifeShader, Shader& skyboxShader, Shader& wheelShader) {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	processInput(window);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//use
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float movementSpeed = 0.1f;
	float movement = deltaTime * movementSpeed;
	if (std::fabs(dir.y) > 1e-4)
		knife.move(Knife::FORWARD, movement * dir.y);
	if (std::fabs(dir.x) > 1e-4)
		knife.move(Knife::RIGHT, movement * dir.x);


	if (current_mode == AUTO) {
		if (auto_mode_t <= 1.0f) {
			autoStride();
		}
		else {
			auto_mode_t = 0.0f;
			current_mode = FREE;
		}
	}

	workpiece.cut(knife.head);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (float)window_width / window_height, 0.1f, 100.0f);
	glm::mat4 model = glm::mat4(1.0f);
	if (!stopRotate) {
		model = glm::rotate(model, (float)glfwGetTime() * 50, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	workpieceShader.use();
	workpieceShader.setMat4("model", model);
	glm::mat4 normalMat = glm::mat3(glm::transpose(glm::inverse(model)));
	workpieceShader.setMat4("view", view);
	workpieceShader.setMat4("projection", projection);
	workpieceShader.setVec3("lightPos", lightPos);
	workpieceShader.setMat3("normalMat", normalMat);
	workpieceShader.setVec3("viewPos", camera.Position);
	particleShader.use();
	model = glm::mat4(1.0f);
	normalMat = glm::mat3(glm::transpose(glm::inverse(model)));
	particleShader.setMat4("model", model);
	particleShader.setMat4("view", view);
	particleShader.setMat4("projection", projection);
	particleShader.setVec3("lightPos", lightPos);
	particleShader.setMat3("normalMat", normalMat);
	particleShader.setVec3("viewPos", camera.Position);

	workpiece.draw(workpieceShader, particleShader);

	knifeShader.use();
	model = glm::mat4(1.0f);
//	model = knife.shift * model;
	normalMat = glm::mat3(glm::transpose(glm::inverse(model)));
	knifeShader.setMat4("model", model);
	knifeShader.setMat4("projection", projection);
	knifeShader.setMat4("view", view);
	knifeShader.setVec3("lightPos", lightPos);
	knifeShader.setMat3("normalMat", normalMat);
	knifeShader.setVec3("viewPos", camera.Position);
	knife.draw(knifeShader);
	
	
	skyboxShader.use();
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, -20.0f, 5.0f));
	skyboxShader.setMat4("model", model);
	skyboxShader.setMat4("view", view);
	skyboxShader.setMat4("projection", projection);
	skybox.draw(skyboxShader);
	 


	model = glm::mat4(1.0f);
	view = curveCamera.GetViewMatrix();
	projection = glm::perspective(glm::radians(45.0f), (float)window_width / window_height, 0.1f, 100.0f);
	wheelShader.use();
	model = glm::mat4(1.0f);
	wheelShader.setMat4("model", model);
	wheelShader.setMat4("view", view);
	wheelShader.setMat4("projection", projection);
	wheel.setInverseMat(glm::inverse(projection * view * model));
	wheel.draw(wheelShader);
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
	Shader knifeShader("./shaders/mainShader.vs", "./shaders/mainShader.fs");
	Shader curveShader("./shaders/curveShader.vs", "./shaders/curveShader.fs");
	Shader skyboxShader("./shaders/skyboxShader.vs", "./shaders/skyboxShader.fs");
	Shader wheelShader("./shaders/curveShader.vs", "./shaders/curveShader.fs");
	workpiece = Workpiece(init_length, init_radius, glm::vec3(-init_length / 2, 0.0f, 0.0f), material);                
	knife = Knife(glm::vec3(-init_length / 2, 0.0f, init_radius), glm::vec3(-init_length / 2 - 0.1f, 0.0f, init_radius + 0.5f), glm::vec3(-init_length / 2 + 0.1f, 0.0f, init_radius + 0.5f), "metal");
	bezierCurve = BezierCurve(glm::vec3(-init_length / 2, 0.0f, 0.0f), glm::vec3(-init_length / 4, 0.0f, 0.0f), glm::vec3(init_length / 4, 0.0f, 0.0f), glm::vec3(init_length / 2, 0.0f, 0.0f));
	skybox = Skybox(0);
	wheel = Wheel(glm::vec3(3.0f, -1.4f, 0.0f));
	
	while (!glfwWindowShouldClose(window)) {
		if (current_mode == FREE || current_mode == AUTO || current_mode == CONSTRAIN) {
			cutMode(window, workpieceShader, particleShader, knifeShader, skyboxShader, wheelShader);
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


	if (current_mode == FREE || current_mode == AUTO || current_mode == CONSTRAIN) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);
		if (current_mode != AUTO) {
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


		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
			if (material == "wood") {
				material = "metal";
				workpiece = Workpiece(init_length, init_radius, glm::vec3(-init_length / 2, 0.0f, 0.0f), material);
			}
			else {
				material = "wood";
				workpiece = Workpiece(init_length, init_radius, glm::vec3(-init_length / 2, 0.0f, 0.0f), material);
			}
			knife = Knife(glm::vec3(-init_length / 2, 0.0f, init_radius), glm::vec3(-init_length / 2 - 0.1f, 0.0f, init_radius + 0.5f), glm::vec3(-init_length / 2 + 0.1f, 0.0f, init_radius + 0.5f), "metal");
			current_mode = FREE;
		}

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			workpiece = Workpiece(init_length, init_radius, glm::vec3(-init_length / 2, 0.0f, 0.0f), material);
			knife = Knife(glm::vec3(-init_length / 2, 0.0f, init_radius), glm::vec3(-init_length / 2 - 0.1f, 0.0f, init_radius + 0.5f), glm::vec3(-init_length / 2 + 0.1f, 0.0f, init_radius + 0.5f), "metal");
			current_mode = FREE;
		}

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			stopRotate = false;
		}
		if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
			stopRotate = true;
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			knife.isConstrain = false;
		}

		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && current_mode == CONSTRAIN) {
			knife = Knife(glm::vec3(-init_length / 2, 0.0f, init_radius), glm::vec3(-init_length / 2 - 0.1f, 0.0f, init_radius + 0.5f), glm::vec3(-init_length / 2 + 0.1f, 0.0f, init_radius + 0.5f), "metal");
			glm::vec3 start = bezierCurve.sample(0.0f);
			knife.reset(glm::vec3(start.x, 0, start.y));
			auto_mode_t = 0.0f;
			current_mode = AUTO;
		}

		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && current_mode == AUTO) {
			auto_mode_t = 2.0f;
		}
	}
	else if (current_mode == DRAW_BEZIER) {
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
			current_mode = CONSTRAIN;
			auto_mode_t = 0.0f;
			glm::vec3 start = bezierCurve.sample(0.0f);
		//	knife.reset(glm::vec3(-init_length / 2, 0.0f, init_radius));
			knife.y_x = bezierCurve.sampleAll(-init_length / 2);
			knife.isConstrain = true;
			knife.startx = -init_length / 2;
		}


		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
			alt_press = true;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_RELEASE) {
			alt_press = false;
		}
		if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
			displayControlPoint = false;
		}
		if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
			displayControlPoint = true;
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

	if (current_mode == FREE || current_mode == CONSTRAIN) {
		camera.ProcessMouseMovement(xoffset, yoffset);
		if (mouse_press) {
			dir = wheel.ProcessMouseMovement(xpos / window_width * 2 - 1.0f, -(ypos / window_height * 2 - 1.0f));
			dir = glm::normalize(dir);
			dir = -dir;
			
		}
	}
	else if (current_mode == DRAW_BEZIER) {
		if (mouse_press && !alt_press) {
			bezierCurve.ProcessMouseMovement(xpos / window_width * 2 - 1.0f, -(ypos / window_height * 2 - 1.0f));
		}
		if (mouse_press && alt_press) {
			bezierCurve.moveAll(xpos / window_width * 2 - 1.0f, -(ypos / window_height * 2 - 1.0f), 
								lastX / window_width * 2 - 1.0f, -(lastY / window_height * 2 - 1.0f));
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
//	if (current_mode == DRAW_BEZIER) {
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
					dir = glm::vec3(0.0f);
					wheel.resetControlPoint();
					break;
				}
			}
//	}
	return;
}