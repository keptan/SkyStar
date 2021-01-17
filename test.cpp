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
#include "systems.h"

using namespace SDL2pp;



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
	SDL sdl(SDL_INIT_VIDEO);


	Window window("demo", 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED,
			640, 480,
			SDL_WINDOW_RESIZABLE);

	Renderer rendr (window, -1, SDL_RENDERER_ACCELERATED);
	WorldSystems world;
	world.registerComponent<pos>();
	world.registerComponent<renderTag>();
	world.registerComponent<animationTag>();
	world.registerComponent<sprite>();
	world.registerComponent<velocity>();

	auto fire	= std::make_shared<Texture>(rendr, DATA_PATH "/flame.png");
	auto wallpaper = std::make_shared<Texture>(rendr, DATA_PATH "/wall.png");

	for(int i = 0; i < 100; i++)
	{
		auto e = world.newEntity();
		world.addComponent<renderTag>(i, {});
		world.addComponent<animationTag>(i, {});
		world.addComponent<sprite>(i, {fire, 16, 8, 1, 0});
		world.addComponent<pos>(i, {std::experimental::randint(0, 640), std::experimental::randint(0, 480)});
		world.addComponent<velocity>(i, {std::experimental::randint(10, 10), std::experimental::randint(0, 0)});
	}


	GameState state; 
	state.time = SDL_GetTicks();

	unsigned int averageFrameTime;

	for(int i = 0; i < 400; i++)
	{
	const auto tick = SDL_GetTicks();
	state.frameTime = tick - state.time;
	state.time = tick;

	sweeper(world, state);
	moveSystem(world, state);
	animationSystem(world, state);
	renderWall(world, state, rendr, wallpaper);
	renderSystem(world, state, rendr);

	state.frameCount++;
	const auto endFrame = SDL_GetTicks();
	const auto frameCost = endFrame - tick;
	averageFrameTime+= frameCost;
	SDL_Delay(frameCost > 16 ? 16 : 16 - frameCost);
	}
	std::cout << averageFrameTime / state.frameCount << '\n';


}





