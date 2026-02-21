#include <iostream>
#include <string>
#include <cmath>

//#define GLEW_STATIC

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



// Shaders
#include "Shader.h"

const GLint WIDTH = 800, HEIGHT = 600;

const float GROUND_Y = -1.5f;
const float CAMERA_EYE_HEIGHT = 1.7f;
const float CAMERA_SPEED = 5.0f;
const float JUMP_VELOCITY = 6.5f;
const float GRAVITY = 17.0f;
const float MOUSE_SENSITIVITY = 0.2f;

float gYaw = -90.0f;
float gPitch = -10.0f;
bool gFirstMouseSample = true;
bool gJumpPressedLastFrame = false;
bool gIsGrounded = true;
double gLastMouseX = 0.0;
double gLastMouseY = 0.0;
float gVerticalVelocity = 0.0f;
glm::vec3 gCameraPos(0.0f, GROUND_Y + CAMERA_EYE_HEIGHT, 8.0f);

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	(void)window;

	if (gFirstMouseSample) {
		gLastMouseX = xpos;
		gLastMouseY = ypos;
		gFirstMouseSample = false;
		return;
	}

	const float xOffset = static_cast<float>(xpos - gLastMouseX);
	const float yOffset = static_cast<float>(gLastMouseY - ypos);
	gLastMouseX = xpos;
	gLastMouseY = ypos;

	gYaw += xOffset * MOUSE_SENSITIVITY;
	gPitch += yOffset * MOUSE_SENSITIVITY;

	if (gPitch > 89.0f) {
		gPitch = 89.0f;
	}
	if (gPitch < -89.0f) {
		gPitch = -89.0f;
	}
}

glm::vec3 getCameraFront()
{
	const float yawRad = glm::radians(gYaw);
	const float pitchRad = glm::radians(gPitch);

	glm::vec3 front(0.0f);
	front.x = std::cos(yawRad) * std::cos(pitchRad);
	front.y = std::sin(pitchRad);
	front.z = std::sin(yawRad) * std::cos(pitchRad);
	return glm::normalize(front);
}

glm::vec3 getHorizontalFront()
{
	glm::vec3 horizontal = getCameraFront();
	horizontal.y = 0.0f;

	if (glm::length(horizontal) < 0.0001f) {
		return glm::vec3(0.0f, 0.0f, -1.0f);
	}

	return glm::normalize(horizontal);
}

void processInput(GLFWwindow* window, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	float speed = CAMERA_SPEED;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		speed *= 1.8f;
	}

	const glm::vec3 horizontalFront = getHorizontalFront();
	const glm::vec3 right = glm::normalize(glm::cross(horizontalFront, glm::vec3(0.0f, 1.0f, 0.0f)));
	const float velocity = speed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		gCameraPos += horizontalFront * velocity;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		gCameraPos -= horizontalFront * velocity;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		gCameraPos -= right * velocity;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		gCameraPos += right * velocity;
	}

	const bool jumpPressedNow = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
	if (jumpPressedNow && !gJumpPressedLastFrame && gIsGrounded) {
		gIsGrounded = false;
		gVerticalVelocity = JUMP_VELOCITY;
	}
	gJumpPressedLastFrame = jumpPressedNow;
}

void updateJump(float deltaTime)
{
	if (gIsGrounded) {
		return;
	}

	gVerticalVelocity -= GRAVITY * deltaTime;
	gCameraPos.y += gVerticalVelocity * deltaTime;

	const float groundEyeY = GROUND_Y + CAMERA_EYE_HEIGHT;
	if (gCameraPos.y <= groundEyeY) {
		gCameraPos.y = groundEyeY;
		gVerticalVelocity = 0.0f;
		gIsGrounded = true;
	}
}


int main() {
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return EXIT_FAILURE;
	}
	// Verificacion de compatibilidad
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecciones y transformaciones basicas", nullptr, nullptr);

	//Verificaci�n de errores de creacion  ventana
	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;

	//Verificaci�n de errores de inicializaci�n de glew

	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to initialise GLEW" << std::endl;
		return EXIT_FAILURE;
	}


	// Define las dimensiones del viewport
	glViewport(0, 0, screenWidth, screenHeight);


	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);

	// enable alpha support
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Build and compile our shader program
#ifdef SHADER_DIR
	const std::string shaderDir = SHADER_DIR;
#else
	const std::string shaderDir = "Shader";
#endif
	const std::string vertexShaderPath = shaderDir + "/core.vs";
	const std::string fragmentShaderPath = shaderDir + "/core.frag";
	Shader ourShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());


	// Set up vertex data (and buffer(s)) and attribute pointers
	// use with Orthographic Projection

	//GLfloat vertices[] = {
 //      -0.5f*500, -0.5f, 0.5f, 1.0f, 0.0f,0.0f,//Front
	//	0.5f * 500, -0.5f * 500, 0.5f * 500,  1.0f, 0.0f,0.0f,
	//	0.5f * 500,  0.5f * 500, 0.5f * 500,  1.0f, 0.0f,0.0f,
	//	0.5f * 500,  0.5f * 500, 0.5f * 500,  1.0f, 0.0f,0.0f,
	//	-0.5f * 500,  0.5f * 500, 0.5f * 500, 1.0f, 0.0f,0.0f,
	//	-0.5f * 500, -0.5f * 500, 0.5f * 500, 1.0f, 0.0f,0.0f,
	//	
	//    -0.5f * 500, -0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,//Back
	//	 0.5f * 500, -0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,
	//	 0.5f * 500,  0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,
	//	 0.5f * 500,  0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,
	//    -0.5f * 500,  0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,
	//    -0.5f * 500, -0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,
	//	
	//	 0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 0.0f,1.0f,
	//	 0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 0.0f,1.0f,
	//	 0.5f * 500,  0.5f * 500, -0.5f * 500,  0.0f, 0.0f,1.0f,
	//	 0.5f * 500,  0.5f * 500, -0.5f * 500,  0.0f, 0.0f,1.0f,
	//	 0.5f * 500,  0.5f * 500,  0.5f * 500,  0.0f, 0.0f,1.0f,
	//	 0.5f * 500,  -0.5f * 500, 0.5f * 500, 0.0f, 0.0f,1.0f,
 //     
	//	-0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 1.0f,0.0f,
	//	-0.5f * 500,  0.5f * 500, -0.5f * 500,  1.0f, 1.0f,0.0f,
	//	-0.5f * 500, -0.5f * 500, -0.5f * 500,  1.0f, 1.0f,0.0f,
	//	-0.5f * 500, -0.5f * 500, -0.5f * 500,  1.0f, 1.0f,0.0f,
	//	-0.5f * 500, -0.5f * 500,  0.5f * 500,  1.0f, 1.0f,0.0f,
	//	-0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 1.0f,0.0f,
	//	
	//	-0.5f * 500, -0.5f * 500, -0.5f * 500, 0.0f, 1.0f,1.0f,
	//	0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 1.0f,1.0f,
	//	0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 1.0f,1.0f,
	//	0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 1.0f,1.0f,
	//	-0.5f * 500, -0.5f * 500,  0.5f * 500, 0.0f, 1.0f,1.0f,
	//	-0.5f * 500, -0.5f * 500, -0.5f * 500, 0.0f, 1.0f,1.0f,
	//	
	//	-0.5f * 500,  0.5f * 500, -0.5f * 500, 1.0f, 0.2f,0.5f,
	//	0.5f * 500,  0.5f * 500, -0.5f * 500,  1.0f, 0.2f,0.5f,
	//	0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 0.2f,0.5f,
	//	0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 0.2f,0.5f,
	//	-0.5f * 500,  0.5f * 500,  0.5f * 500, 1.0f, 0.2f,0.5f,
	//	-0.5f * 500,  0.5f * 500, -0.5f * 500, 1.0f, 0.2f,0.5f,
	//};
	

	// use with Perspective Projection
	float vertices[] = {
		-0.5f, -0.5f, 0.5f, 1.0f, 0.0f,0.0f,//Front
		0.5f, -0.5f, 0.5f,  1.0f, 0.0f,0.0f,
		0.5f,  0.5f, 0.5f,  1.0f, 0.0f,0.0f,
		0.5f,  0.5f, 0.5f,  1.0f, 0.0f,0.0f,
		-0.5f,  0.5f, 0.5f, 1.0f, 0.0f,0.0f,
		-0.5f, -0.5f, 0.5f, 1.0f, 0.0f,0.0f,
		
	    -0.5f, -0.5f,-0.5f, 0.0f, 1.0f,0.0f,//Back
		 0.5f, -0.5f,-0.5f, 0.0f, 1.0f,0.0f,
		 0.5f,  0.5f,-0.5f, 0.0f, 1.0f,0.0f,
		 0.5f,  0.5f,-0.5f, 0.0f, 1.0f,0.0f,
	    -0.5f,  0.5f,-0.5f, 0.0f, 1.0f,0.0f,
	    -0.5f, -0.5f,-0.5f, 0.0f, 1.0f,0.0f,
		
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 0.0f,1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f,1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f,1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 0.0f,1.0f,
		 0.5f,  -0.5f, 0.5f, 0.0f, 0.0f,1.0f,
      
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f,0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,0.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f,0.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f,0.0f,
		-0.5f, -0.5f,  0.5f,  1.0f, 1.0f,0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f,0.0f,
		
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, 1.0f,1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, 1.0f,1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 1.0f,1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,1.0f,
		
		-0.5f,  0.5f, -0.5f, 1.0f, 0.2f,0.5f,
		0.5f,  0.5f, -0.5f,  1.0f, 0.2f,0.5f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.2f,0.5f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.2f,0.5f,
		-0.5f,  0.5f,  0.5f, 1.0f, 0.2f,0.5f,
		-0.5f,  0.5f, -0.5f, 1.0f, 0.2f,0.5f,
	};

	const float groundHalfSize = 60.0f;
	float groundVertices[] = {
		-groundHalfSize, GROUND_Y, -groundHalfSize, 0.70f, 0.70f, 0.70f,
		 groundHalfSize, GROUND_Y, -groundHalfSize, 0.70f, 0.70f, 0.70f,
		 groundHalfSize, GROUND_Y,  groundHalfSize, 0.65f, 0.65f, 0.65f,

		 groundHalfSize, GROUND_Y,  groundHalfSize, 0.65f, 0.65f, 0.65f,
		-groundHalfSize, GROUND_Y,  groundHalfSize, 0.65f, 0.65f, 0.65f,
		-groundHalfSize, GROUND_Y, -groundHalfSize, 0.70f, 0.70f, 0.70f,
	};

	GLuint VBO, VAO;
	GLuint groundVBO, groundVAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &groundVAO);
	glGenBuffers(1, &groundVBO);
	//glGenBuffers(1, &EBO);

	// Enlazar  Vertex Array Object
	glBindVertexArray(VAO);

	//2.- Copiamos nuestros arreglo de vertices en un buffer de vertices para que OpenGL lo use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 3.Copiamos nuestro arreglo de indices en  un elemento del buffer para que OpenGL lo use
	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

	// 4. Despues colocamos las caracteristicas de los vertices

	//Posicion
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	//Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)

	glBindVertexArray(groundVAO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);//FOV, Radio de aspecto,znear,zfar
	//projection = glm::ortho(0.0f, (GLfloat)screenWidth, 0.0f, (GLfloat)screenHeight, 0.1f, 1000.0f);//Izq,Der,Fondo,Alto,Cercania,Lejania

	GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
	GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
	GLint projecLoc = glGetUniformLocation(ourShader.Program, "projection");

	float lastFrameTime = static_cast<float>(glfwGetTime());
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		const float currentTime = static_cast<float>(glfwGetTime());
		float deltaTime = currentTime - lastFrameTime;
		if (deltaTime > 0.05f) {
			deltaTime = 0.05f;
		}
		lastFrameTime = currentTime;
		processInput(window, deltaTime);
		updateJump(deltaTime);
		const float t = currentTime;

		// Render
		// Clear the colorbuffer
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.Use();
		const glm::mat4 view = glm::lookAt(gCameraPos, gCameraPos + getCameraFront(), glm::vec3(0.0f, 1.0f, 0.0f));

		glUniformMatrix4fv(projecLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// Ground plane
		glm::mat4 model = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(groundVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Animated models (positions stay fixed)
		model = glm::mat4(1);
		model = glm::rotate(model, t * glm::radians(45.0f), glm::vec3(5.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(3.0f, 0.0f, 0.0f));
		model = glm::rotate(model, t * glm::radians(65.0f), glm::vec3(1.0f, 0.5f, 0.3f));
		model = glm::rotate(model, t * glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f, 0.3f, 2.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-3.9f, 0.0f, 0.0f));
		model = glm::rotate(model, t * glm::radians(35.0f), glm::vec3(0.5f, 0.5f, 0.8f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &groundVAO);
	glDeleteBuffers(1, &groundVBO);


	glfwTerminate();
	return EXIT_SUCCESS;

  

}
