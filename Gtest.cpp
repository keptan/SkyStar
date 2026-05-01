#include <glad/gl.h>
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

	while (isRunning) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				isRunning = false;
			}
		}

		// Render Frame
		glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Swap buffers using the raw window pointer
		SDL_GL_SwapWindow(window.Get());
	}
	return 0;
}