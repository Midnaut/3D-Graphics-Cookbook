#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "imgui.cpp"
#include "imgui_demo.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <imgui.h>
//#include <imgui_impl_glfw.h>
//#include <imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <cstdio>
#include <cstdlib>

#include <iostream>

#pragma region SHADER_STRINGS

static const char* vertexShaderString = R"(

#version 460 core
layout(std140, binding = 0) uniform PerFrameData
{
	uniform mat4 MVP;
	uniform int isWireframe;
};
layout (location = 0) out vec3 color;
layout (location = 1) out vec2 texCoord;

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

const vec2 texCoords[8] = vec2[8] (
		vec2(0.0, 0.0),  
		vec2(1.0, 0.0),  
		vec2(1.0, 1.0),  
		vec2(0.0, 1.0),  
		vec2(0.0, 0.0),  
		vec2(1.0, 0.0), 
		vec2(1.0, 1.0),  
		vec2(0.0, 1.0)
);

const int indices[36] = int[36](
	// front
	0, 1, 2, 
	2, 3, 0,
	// right
	1, 5, 6, 
	6, 2, 1,
	// back
	7, 6, 5, 
	5, 4, 7,
	// left
	4, 0, 3, 
	3, 7, 4,
	// bottom
	4, 5, 1, 
	1, 0, 4,
	// top
	3, 2, 6, 
	6, 7, 3s
);

const int uvLookup[6] = int[6](
	0, 1, 2, 
	2, 3, 0
);

void main()
{
	int idx = indices[gl_VertexID];
	gl_Position = MVP * vec4(pos[idx], 1.0);
	color = isWireframe > 0 ? vec3(0.0) : col[idx];

	int uvIdx = uvLookup[gl_VertexID % 6];
	texCoord = texCoords[uvIdx];
}

)";

static const char* guiVertexShaderString = R"(

#version 460 core
layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec4 Color;

layout (std140, binding = 0) uniform PerFrameData
{
	uniform mat4 MVP;
};

out vec2 Frag_UV;
out vec4 Frag_Color;

void main()
{
	Frag_UV = UV;
	Frag_Color = Color;
	gl_Position = MVP * vec4(Position.xy,0,1);
}

)";

static const char* fragmentShaderString = R"(

#version 460 core
layout (location=0) in vec3 color;
layout (location=1) in vec2 texCoord;
layout (location=0) out vec4 out_FragColor;
uniform sampler2D texture0;
void main()
{
	vec4 texCol =  texture(texture0, texCoord);
	vec4 vertCol = vec4(color , 1.0);
	out_FragColor =  texCol * vertCol;
};

)";

static const char* guiFragmentShaderString = R"(

#version 460 core
in vec2 Frag_UV;
in vec4 Frag_Color;
layout (binding = 0) uniform sampler2D Texture;
layout (location = 0) out vec4 out_Color;

void main()
{
	out_Color = Frag_Color * texture(
	Texture, Frag_UV.st);
}

)";

#pragma endregion

struct Resources {
	GLFWwindow* window;
	// Buffers
	GLuint VAO;
	GLuint perFrameDataBuffer;
	GLsizeiptr kBufferSize;
	// Shaders + program
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;
	// MVP
	glm::mat4 matrix;
	glm::mat4 perspective;
	// image
	GLuint texture;
	uint8_t* img;
	int imgW;
	int imgH;
	int imgComp;
	//imgui
	GLuint guiVAO;
	GLuint guiHandleVBO;
	GLuint guiHandleElements;
	GLuint guiVertexShader;
	GLuint guiFragmentShader;
	GLuint guiProgram;
	GLuint guiTexture;
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
			if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
			{
				int width, height;

				glfwGetFramebufferSize(window, &width, &height);

				uint8_t* imgPtr = (uint8_t*)malloc( width * height * 4);

				glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imgPtr);

				stbi_write_png("screenshot.png", width, height, 4, imgPtr, 0);
				free(imgPtr);
			}
		}
	);

	glfwMakeContextCurrent(resources->window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval(1);

}

void LoadTexture(Resources* resources) {
	const char* filename = "C:/DEV/3D-Graphics-Cookbook-Projects/data/ch2_sample3_STB.jpg";
	resources->img = stbi_load(filename, &resources->imgW, &resources->imgH, &resources->imgComp, 3);
	
	if (stbi_failure_reason())
		std::cout << stbi_failure_reason();

	glCreateTextures(GL_TEXTURE_2D, 1, &resources->texture);
	glTextureParameteri(resources->texture, GL_TEXTURE_MAX_LEVEL, 0);
	glTextureParameteri(resources->texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(resources->texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureStorage2D(resources->texture, 1, GL_RGB8, resources->imgW, resources->imgH);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureSubImage2D(resources->texture, 0, 0, 0, resources->imgW, resources->imgH, GL_RGB, GL_UNSIGNED_BYTE, resources->img);
	glBindTextures(0, 1, &resources->texture);

}

void CreateVAO(Resources* resources) {
	glCreateVertexArrays(1, &resources->VAO);
	glBindVertexArray(resources->VAO);
}

void CreateGUIData(Resources* resources) {
	glCreateVertexArrays(1, &resources->guiVAO);
	glCreateBuffers(1, &resources->guiHandleVBO);
	glNamedBufferStorage(resources->guiHandleVBO, 256 * 1024, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glCreateBuffers(1, &resources->guiHandleElements);
	glNamedBufferStorage(resources->guiHandleElements, 256 * 1024, nullptr, GL_DYNAMIC_STORAGE_BIT);

	// Setup Geo data
	glVertexArrayElementBuffer(resources->guiVAO, resources->guiHandleElements);
	glVertexArrayVertexBuffer(resources->guiVAO, 0, resources->guiHandleVBO, 0, sizeof(ImDrawVert));
	glEnableVertexArrayAttrib(resources->guiVAO, 0);
	glEnableVertexArrayAttrib(resources->guiVAO, 1);
	glEnableVertexArrayAttrib(resources->guiVAO, 2);

	glVertexArrayAttribFormat(resources->guiVAO, 0, 2, GL_FLOAT, GL_FALSE, IM_OFFSETOF(ImDrawVert, pos));
	glVertexArrayAttribFormat(resources->guiVAO, 1, 2, GL_FLOAT, GL_FALSE, IM_OFFSETOF(ImDrawVert, uv));
	glVertexArrayAttribFormat(resources->guiVAO, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, IM_OFFSETOF(ImDrawVert, col));

	glVertexArrayAttribBinding(resources->guiVAO, 0, 0);
	glVertexArrayAttribBinding(resources->guiVAO, 1, 0);
	glVertexArrayAttribBinding(resources->guiVAO, 2, 0);

	glBindVertexArray(resources->guiVAO);
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

}

void CreateGUIShaders(Resources* resources) {
	resources->guiVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(resources->guiVertexShader, 1, &vertexShaderString, nullptr);
	glCompileShader(resources->guiVertexShader);

	resources->guiFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(resources->guiFragmentShader, 1, &fragmentShaderString, nullptr);
	glCompileShader(resources->guiFragmentShader);

	resources->guiProgram = glCreateProgram();
	glAttachShader(resources->guiProgram, resources->vertexShader);
	glAttachShader(resources->guiProgram, resources->fragmentShader);

	glLinkProgram(resources->guiProgram);
}

void InitImGui(Resources* resources) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

	ImFontConfig cfg = ImFontConfig();
	cfg.FontDataOwnedByAtlas = false;
	cfg.RasterizerMultiply = 1.5f;
	// window height x 32 lines
	cfg.SizePixels = 768.0f / 32.0f;
	cfg.PixelSnapH = true;
	cfg.OversampleH = 4;
	cfg.OversampleV = 4;

	ImFont* Font = io.Fonts->AddFontFromFileTTF("C:/DEV/3D-Graphics-Cookbook-Projects/data/OpenSans-Light.ttf", cfg.SizePixels, &cfg);

	unsigned char* pixels = nullptr;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	glCreateTextures(GL_TEXTURE_2D, 1, &resources->guiTexture);
	glTextureParameteri(resources->guiTexture, GL_TEXTURE_MAX_LEVEL, 0);
	glTextureParameteri(resources->guiTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(resources->guiTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureStorage2D(resources->guiTexture, 1, GL_RGBA8, width, height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureSubImage2D(resources->guiTexture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindTextures(0, 1, &resources->guiTexture);

	io.Fonts->TexID = (ImTextureID)(intptr_t)resources->guiTexture;
	io.FontDefault = Font;
	io.DisplayFramebufferScale = ImVec2(1, 1);
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

void PrepGLForMesh() {
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, -1.0f);
}

void PrepGLForImGui() {
	glDisable(GL_POLYGON_OFFSET_LINE);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
}

void Render(Resources* resources) {

	int width;
	int height;
	glfwGetFramebufferSize(resources->window, &width, &height);

	// Clear the screen
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Render Mesh
	PrepGLForMesh();

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


	// Render GUI
	PrepGLForImGui();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
	ImGui::Render();

	// Swap Buffers
	glScissor(0, 0, width, height);
	glfwSwapBuffers(resources->window);
	glfwPollEvents();
}



void Cleanup(Resources* resources) {
	glDeleteTextures(1, &resources->texture);
	glDeleteBuffers(1, &resources->perFrameDataBuffer);
	glDeleteProgram(resources->program);
	glDeleteShader(resources->fragmentShader);
	glDeleteShader(resources->vertexShader);
	glDeleteVertexArrays(1, &resources->VAO);
}

void TerminateOpenGL(GLFWwindow* window) {
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}


int main() {

	Resources resources;

	InitOpenGL(&resources);
	LoadTexture(&resources);
	CreateVAO(&resources);
	CreateGUIData(&resources);
	CreateDynamicBuffers(&resources);
	CreateShaders(&resources);
	CreateGUIShaders(&resources);
	InitImGui(&resources);

	while (!glfwWindowShouldClose(resources.window)) {
		Update(&resources);
		Render(&resources);
	}

	Cleanup(&resources);
	TerminateOpenGL(resources.window);

	return 0;
}
