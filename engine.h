#pragma once
#include "star.h"
#include <SDL2pp/Renderer.hh>
#include <SDL2pp/SDL2pp.hh>
#include <filesystem>

enum class InputMask
{
	None = 0x00,
	Up	 = 1 << 1,
	Down = 1 << 2,
	Left = 1 << 3,
	Right = 1 << 4,
	Attack = 1 << 5,
	Quit = 1 << 6,
	LShift = 1 << 7,
	PKey   = 1 << 8,
	Space  = 1 << 9,
};

inline InputMask operator | (InputMask lhs, InputMask rhs)
{
	using T = std::underlying_type_t<InputMask>;
	return static_cast<InputMask>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline InputMask operator & (InputMask lhs, InputMask rhs)
{
	using T = std::underlying_type_t<InputMask>;
	return static_cast<InputMask>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline InputMask& operator ^= (InputMask& lhs, InputMask rhs)
{	
	using T = std::underlying_type_t<InputMask>;
	lhs = static_cast<InputMask>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
	return lhs;
}

inline InputMask& operator |= (InputMask& lhs, InputMask rhs)
{	
	using T = std::underlying_type_t<InputMask>;
	lhs = static_cast<InputMask>(static_cast<T>(lhs) | static_cast<T>(rhs));
	return lhs;
}

struct GameState
{
	InputMask input = InputMask::None;

	unsigned int frameCount = 0;
	unsigned long int time = 0;
	unsigned int frameTime = 0;
};


void sweeper (WorldSystems& world, GameState& state)
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		 if (event.type == SDL_KEYDOWN) 
		 {
				switch (event.key.keysym.sym) 
				{
					case SDLK_w: state.input |= InputMask::Up; break;
					case SDLK_a: state.input |= InputMask::Left; break;
					case SDLK_s: state.input |= InputMask::Down; break;
					case SDLK_d: state.input |= InputMask::Right; break;
					case SDLK_q: state.input |= InputMask::Quit; break;
					case SDLK_LSHIFT: state.input |= InputMask::LShift; break;
					case SDLK_p: state.input |= InputMask::PKey; break;
					case SDLK_SPACE: state.input |= InputMask::Space; break;
				}

		} 
		else if (event.type == SDL_KEYUP)
		{
				switch (event.key.keysym.sym)
				{
					case SDLK_w: state.input ^= InputMask::Up; break;
					case SDLK_a: state.input ^= InputMask::Left; break;
					case SDLK_s: state.input ^= InputMask::Down; break;
					case SDLK_d: state.input ^= InputMask::Right; break;
					case SDLK_q: state.input ^= InputMask::Quit; break;
					case SDLK_LSHIFT: state.input ^= InputMask::LShift; break;
					case SDLK_p: state.input ^= InputMask::PKey; break;
					case SDLK_SPACE: state.input ^= InputMask::Space; break;
				}
		}
	}
}

struct Graphics
{
	std::unordered_map< std::string, std::shared_ptr<SDL2pp::Texture>> library;
	Graphics (void) {
	for(const auto& dir : std::filesystem::recursive_directory_iterator(DATA_PATH)) std::cout << dir << std::endl;}

};

struct GameWindow
{
	SDL2pp::SDL   	sdl;
	SDL2pp::SDLTTF 	sdl_ttf;
	SDL2pp::Window	window;
	SDL2pp::Renderer rendr;


	GameWindow (void)
		: sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO),
		  window("Demo..", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			 1280, 960,
			 SDL_WINDOW_RESIZABLE),
		  rendr(window, -1, SDL_RENDERER_ACCELERATED)
		{}
};




