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
	SDL2pp::SDL sdl(SDL_INIT_VIDEO);


	SDL2pp::Window window("demo", 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED,
			640, 480,
			SDL_WINDOW_RESIZABLE);

	SDL2pp::Renderer rendr (window, -1, SDL_RENDERER_ACCELERATED);
	WorldSystems world;
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

	auto e = world.newEntity();
	world.addComponent<renderTag>(e, {});
	world.addComponent<animationTag>(e, {});
	world.addComponent<sprite>(e, {lala, 37, 21, 7, 0});
	world.addComponent<pos>(e, {std::experimental::randint(0, 640), std::experimental::randint(0, 480)});
	world.addComponent<velocity>(e, {0, 0});
	world.addComponent<playerTag>(e, {});
	world.addComponent<collision>(e, {100});
	world.addComponent<path>(e, {0, 7000, 1, {{25, 100}, {400, 500}, {200, 200}, {10, 10}, {480, 480}, {10, 160}, {160, 10}}});


	for(int x = 0; x < 640; x += 30)
	{
	
		for(int y = 0; y < 480; y += 30)
		{

		e = world.newEntity();
		world.addComponent<renderTag>(e, {});
		world.addComponent<animationTag>(e, {});
		world.addComponent<sprite>(e, {fire, 16, 8, 1, 0});
		world.addComponent<pos>(e, {std::experimental::randint(0, 640), std::experimental::randint(0, 480)});
		world.addComponent<velocity>(e, {std::experimental::randint(-15, 15), std::experimental::randint(50, 140)});
		world.addComponent<collision>(e, {100});
		}
	}


	GameState state;
	SpaceGrid grid (world, 40, 640, 480);
	state.time = SDL_GetTicks();

	unsigned int averageFrameTime;

	for(int i = 0; i < 600; i++)
	{
	const auto tick = SDL_GetTicks();
	state.frameTime = tick - state.time;
	state.time = tick;

	grid.regen(world);

	sweeper(world, state);
	playerMove(world, state);
	moveSystem(world, state);
	pathSystem(world, state);
	animationSystem(world, state);
	collisionSphere(world, state, grid, greenFire, fire);
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





