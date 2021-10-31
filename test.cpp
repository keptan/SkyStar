#include <SDL2pp/Renderer.hh>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <experimental/random>
#include <map>

#include <SDL2pp/SDL2pp.hh>
#include <random>
#include <chrono>
#include <type_traits>
#include "components.h"
#include "systems.h"
#include "entities.h"

template <typename F>
unsigned int time (const F f)
{
	auto start = std::chrono::high_resolution_clock::now();
	f();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
	return duration.count();
};

auto main (void) -> int 
{
	SDL2pp::SDL sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL2pp::SDLTTF sdl_ttf;


	SDL2pp::Window window("demo", 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED,
			1280, 960,
			SDL_WINDOW_RESIZABLE);

	SDL2pp::Renderer rendr (window, -1, SDL_RENDERER_ACCELERATED);

	WorldSystems world;

	GameState state;
	state.time = SDL_GetTicks();

	b2Vec2 gravity (0.0f, 0.0f);
	b2World space (gravity);

	b2BodyDef ground;
	ground.position.Set(0.0f, -10.0f);


	world.registerComponent<pos>();
	world.registerComponent<renderTag>();
	world.registerComponent<animationTag>();
	world.registerComponent<sprite>();
	world.registerComponent<velocity>();
	world.registerComponent<playerTag>();
	world.registerComponent<collision>();
	world.registerComponent<path>();

	auto lala	= std::make_shared<SDL2pp::Texture>(rendr, DATA_PATH "/lala_flying.png");
	auto fire	= std::make_shared<SDL2pp::Texture>(rendr, DATA_PATH "/flame.png");
	auto greenFire	= std::make_shared<SDL2pp::Texture>(rendr, DATA_PATH "/greenFlame.png");
	auto wallpaper = std::make_shared<SDL2pp::Texture>(rendr, DATA_PATH "/wall.png");

	auto e = player(world);
	world.addComponent<sprite>(e, {lala, 37, 21, 7, 0});

	for(int x = 0; x < 640; x += 30)
	{
		for(int y = 0; y < 480; y += 30)
		{
			e = fireball(world, space);
			world.addComponent<sprite>(e, {fire, 16, 8, 1, 0});
		}
	}

	unsigned int averageFrameTime;

	for(int i = 0;; i++)
	{
	const auto tick = SDL_GetTicks();
	state.frameTime = tick - state.time;
	state.time = tick;

	if ((state.input & InputMask::Quit) == InputMask::Quit) break;

	sweeper(world, state);
	playerMove(world, state);
	pathSystem(world, state);
	moveSystem(world, state);
	animationSystem(world, state);
	renderWall(world, state, rendr, wallpaper);
	renderSystem(world, state, rendr);

	space.Step(1.0f / 60.0f, 6, 2);
	boxSystem(world, state);

	state.frameCount++;
	const auto endFrame = SDL_GetTicks();
	const auto frameCost = endFrame - tick;
	averageFrameTime+= frameCost;
	SDL_Delay(frameCost >= 16 ? 0 : 16 - frameCost);
	}
	std::cout << averageFrameTime / state.frameCount << '\n';
}
