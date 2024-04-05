#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <experimental/random>
#include <map>

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

	WorldSystems world;
	GameWindow window;
	GameState state;
	state.time = SDL_GetTicks();

	auto lala	= std::make_shared<SDL2pp::Texture>(window.rendr, DATA_PATH "/lala_flying.png");
	auto fire	= std::make_shared<SDL2pp::Texture>(window.rendr, DATA_PATH "/flame.png");
	auto greenFire	= std::make_shared<SDL2pp::Texture>(window.rendr, DATA_PATH "/greenFlame.png");
	auto wallpaper = std::make_shared<SDL2pp::Texture>(window.rendr, DATA_PATH "/wall.png");

	const auto target  = player(world);
	world.addComponent<sprite>(target, {lala, 37, 21, 7, 0});


	auto e = fireball(world, state);
	world.addComponent<sprite>(e, {fire, 16, 8, 1, 0});
		

	unsigned int averageFrameTime;

	for(int i = 0;; i++)
	{
		const auto tick = SDL_GetTicks();
		state.frameTime = tick - state.time;
		state.time = tick;

		if ((state.input & InputMask::Quit) == InputMask::Quit) break;
		if ((state.input & InputMask::PKey) == InputMask::PKey)
		{
			for(int i = 0; i < 25; i++)
			{
				auto e = fireball(world, state);
				world.addComponent<sprite>(e, {fire, 16, 8, 1, 0});
				std::cout << world.eCount() << std::endl;
			}
		}

		sweeper(world, state);
		playerMove(world, state);
		pathSystem(world, state);
		animationSystem(world, state);
		renderWall(world, state, window.rendr, wallpaper);
		renderSystem(world, state, window.rendr);
		velocitySystem(world, state);
		outOfBounds(world, state);
		pCallbackSystem(world, state);

		state.frameCount++;
		const auto endFrame = SDL_GetTicks();
		const auto frameCost = endFrame - tick;
		averageFrameTime+= frameCost;
		SDL_Delay(frameCost >= 16 ? 0 : 16 - frameCost);
	}
	std::cout << averageFrameTime / state.frameCount << '\n';
}
