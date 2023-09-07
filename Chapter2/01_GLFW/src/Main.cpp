
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>

#pragma region SHADER_STRINGS

static const char* vertexShaderString = R"(

#version 460 core
layout (location=0) out vec3 color;
const vec2 pos[3] = vec2[3] (
	vec2(-0.6, -0.4),
	vec2(0.6, -0.4),
	vec2(0.0, 0.6)
);

const vec3 col[3] = vec3[3] (
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

void main() {
	gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
	color = col[gl_VertexID];
}

)";


static const char* fragmentShaderString = R"(

#version 460 core
layout (location=0) in vec3 color;
layout (location=0) out vec4 out_FragColor;

void main() {
	out_FragColor = vec4(color, 1.0);
}

)";

#pragma endregion

struct Resources {
	GLFWwindow* window;
	GLuint VAO;
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;
};


void InitOpenGL(Resources* resources) {
	glfwSetErrorCallback([](int error, const char* description) {
		fprintf(stderr, "Error: %s\n", description);
		});

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	resources->window = glfwCreateWindow(1024, 768, "Simple Example", nullptr, nullptr);

	if (!resources->window) {
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(
		resources->window,
		[](GLFWwindow* window, int key, int scancode, int action, int mods) {
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
		}
	);

	glfwMakeContextCurrent(resources->window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval(1);

}

void CreateVAO(Resources* resources) {
	glCreateVertexArrays(1, &resources->VAO);
	glBindVertexArray(resources->VAO);
}

void CreateShaders(Resources* resources) {
	resources->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(resources->vertexShader, 1, &vertexShaderString, nullptr);
	glCompileShader(resources->vertexShader);

	resources->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(resources->fragmentShader, 1, &fragmentShaderString, nullptr);
	glCompileShader(resources->fragmentShader);

	resources->program = glCreateProgram();
	glAttachShader(resources->program, resources->vertexShader);
	glAttachShader(resources->program, resources->fragmentShader);

	glLinkProgram(resources->program);
	glUseProgram(resources->program);
}

void Update(GLFWwindow* window) {

	// resize event
	int width;
	int height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);
}

void Render(GLFWwindow* window) {

	// Clear the screen
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Swap Buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Cleanup(Resources* resources) {
	glDeleteProgram(resources->program);
	glDeleteShader(resources->fragmentShader);
	glDeleteShader(resources->vertexShader);
	glDeleteVertexArrays(1, &resources->VAO);
}

void TerminateOpenGL(GLFWwindow* window) {
	glfwDestroyWindow(window);
	glfwTerminate();
}


int main() {

	Resources resources;

	InitOpenGL(&resources);
	CreateVAO(&resources);
	CreateShaders(&resources);

	while (!glfwWindowShouldClose(resources.window)) {
		Update(resources.window);
		Render(resources.window);
	}

	Cleanup(&resources);
	TerminateOpenGL(resources.window);

	return 0;
}