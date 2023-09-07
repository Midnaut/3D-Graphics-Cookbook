
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cstdio>
#include <cstdlib>

#pragma region SHADER_STRINGS

static const char* vertexShaderString = R"(

#version 460 core
layout(std140, binding = 0) uniform PerFrameData
{
	uniform mat4 MVP;
	uniform int isWireframe;
};
layout (location=0) out vec3 color;
const vec3 pos[8] = vec3[8](
	vec3(-1.0,-1.0, 1.0),
	vec3( 1.0,-1.0, 1.0),
	vec3( 1.0, 1.0, 1.0),
	vec3(-1.0, 1.0, 1.0),

	vec3(-1.0,-1.0,-1.0),
	vec3( 1.0,-1.0,-1.0),
	vec3( 1.0, 1.0,-1.0),
	vec3(-1.0, 1.0,-1.0)
);
const vec3 col[8] = vec3[8](
	vec3( 1.0, 0.0, 0.0),
	vec3( 0.0, 1.0, 0.0),
	vec3( 0.0, 0.0, 1.0),
	vec3( 1.0, 1.0, 0.0),

	vec3( 1.0, 1.0, 0.0),
	vec3( 0.0, 0.0, 1.0),
	vec3( 0.0, 1.0, 0.0),
	vec3( 1.0, 0.0, 0.0)
);
const int indices[36] = int[36](
	// front
	0, 1, 2, 2, 3, 0,
	// right
	1, 5, 6, 6, 2, 1,
	// back
	7, 6, 5, 5, 4, 7,
	// left
	4, 0, 3, 3, 7, 4,
	// bottom
	4, 5, 1, 1, 0, 4,
	// top
	3, 2, 6, 6, 7, 3
);
void main()
{
	int idx = indices[gl_VertexID];
	gl_Position = MVP * vec4(pos[idx], 1.0);
	color = isWireframe > 0 ? vec3(0.0) : col[idx];
}

)";


static const char* fragmentShaderString = R"(

#version 460 core
layout (location=0) in vec3 color;
layout (location=0) out vec4 out_FragColor;
void main()
{
	out_FragColor = vec4(color, 1.0);
};

)";

#pragma endregion

struct Resources {
	GLFWwindow* window;
	GLuint VAO;
	GLuint perFrameDataBuffer;
	GLsizeiptr kBufferSize;
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;
	glm::mat4 matrix;
	glm::mat4 perspective;
};

struct PerFrameData {
	glm::mat4 mvp;
	int isWireFrame;
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
		glfwTerminate();
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

void CreateDynamicBuffers(Resources* resources) {
	resources->kBufferSize = sizeof(PerFrameData);
	glCreateBuffers(1, &resources->perFrameDataBuffer);
	glNamedBufferStorage(resources->perFrameDataBuffer, resources->kBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, resources->perFrameDataBuffer, 0, resources->kBufferSize);
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

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, -1.0f);

}

void Update(Resources* resources) {



	// resize event
	int width;
	int height;
	glfwGetFramebufferSize(resources->window, &width, &height);
	
	const float ratio = width / (float)height;
	glViewport(0, 0, width, height);

	//rotate the transformation matix
	resources->matrix = glm::rotate(
		glm::translate(glm::mat4(1.0f), glm::vec3( 0.0f, 0.0f, -3.5f)),
		(float)glfwGetTime(), 
		glm::vec3(1.0f, 1.0f, 1.0f)
	);

	resources->perspective = glm::perspective(45.0f, ratio,  0.1f, 10000.0f);

	

}

void Render(Resources* resources) {

	// Clear the screen
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Render logic
	PerFrameData frameData = {
		.mvp = resources->perspective * resources->matrix,
		.isWireFrame = false
	};

	glNamedBufferSubData(resources->perFrameDataBuffer, 0, resources->kBufferSize, &frameData);
	// Draw polygon
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	frameData.isWireFrame = true;
	glNamedBufferSubData(resources->perFrameDataBuffer, 0, resources->kBufferSize, &frameData);
	// Draw wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// Swap Buffers
	glfwSwapBuffers(resources->window);
	glfwPollEvents();
}

void Cleanup(Resources* resources) {
	glDeleteBuffers(1, &resources->perFrameDataBuffer);
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
	CreateDynamicBuffers(&resources);
	CreateShaders(&resources);

	while (!glfwWindowShouldClose(resources.window)) {
		Update(&resources);
		Render(&resources);
	}

	Cleanup(&resources);
	TerminateOpenGL(resources.window);

	return 0;
}