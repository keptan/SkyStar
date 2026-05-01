#include <glad/gl.h>
#include "graphics/triangle.h"
#include "systems.h"
#include "engine.h"




struct GlContextDeleter
{
	void operator ()(void* ctx) const
	{
		if (ctx)
			SDL_GL_DeleteContext(static_cast<SDL_GLContext>(ctx));
	}
};
using GContextPtr = std::unique_ptr<void, GlContextDeleter>;

auto main (void) -> int
{
	SDL2pp::SDL sdl(SDL_INIT_VIDEO);

	//Set OpenGL Attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL2pp::Window window(
				"Skystar opengl",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				1280,
				720,
				SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
		  );

	GContextPtr glContext(SDL_GL_CreateContext(window.Get()));
	if (!glContext) {
		throw SDL2pp::Exception("Failed to create OpenGL Context");
	}

	gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);


	bool isRunning = true;
	SDL_Event event;

	//we want to make a triangle, first we will make a triangle
	float triangle[] =
	{
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f, 0.5f, 0.0f
	};


	//now lets do an inline shader cause why not. This is the simplest possible vertex shader
	//it just moves the data into the vec4 format
	const char *vertexShaderSource = "#version 330 core\n"
	 "layout (location = 0) in vec3 aPos;\n"
	 "void main()\n"
	 "{\n"
	 "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	 "}\0";

	//assigned an ID, and compile our string shader into it
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	//and yes we will check if it worked or not
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::print("shader compile faile: {}", infoLog);
	}

	//now the fragment shader, the fragment shader just tells what color to make the pixels
	//we are making an RGBA color, there are 16 million options

	const char *fragmentShaderSource = "#version 330 core\n"
	"out vec4 FragColor;\n"
	"void main ()\n"
	"{\n"
	"  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
	"}\0";

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::print("frag shader compile fail: {}", infoLog);
	}

	//now we combine the shaders into a shader program
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::print("shader program link fail: {}", infoLog);
	}

	//use or activate it or whatever
	glUseProgram(shaderProgram);
	//we dont need the shader objects no more
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//next we want to make a vertex buffer which is where in the GPU the triangle will go
	//and a VAO which stores attribute calls that we make about the data we will feed the buffer
	unsigned int VBO, VAO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO); //make an ID for the buffer

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //bind to current array buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
	//we put triangle in the buffer, and tell it to put it in slow memory cause we aren't going to change it

	//next we give opengl info about our triangle object, about the datalayout
	//stored in the VAO
	glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//we dont need the VAO or VBO bound, we're finished editing these two for now
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	while (isRunning) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				isRunning = false;
			}
		}

		// Render Frame
		glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		// Swap buffers using the raw window pointer
		SDL_GL_SwapWindow(window.Get());
	}
	return 0;
}