#include <SDL2pp/Renderer.hh>
#include <iostream>
#include <exception>
#include <vector>
#include <unordered_set>
#include <string>
#include <experimental/random>

#include <SDL2pp/SDL2pp.hh>
#include "entity.h"
#include <chrono>

using namespace SDL2pp;

void sdlTest (void)
{
	SDL sdl(SDL_INIT_VIDEO);

	Window window("demo", 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED,
			640, 480,
			SDL_WINDOW_RESIZABLE);

	Renderer rendr (window, -1, SDL_RENDERER_ACCELERATED);
	Texture ssheet (rendr, DATA_PATH "/lala.png");
	int vc = rendr.GetOutputHeight() / 2;
	int hc = rendr.GetOutputWidth() / 2;

	rendr.Clear();

	rendr.Copy(
			ssheet, 
			Rect(0, 0, 32, 32), 
			Rect(hc - 16, vc - 16, 32, 32));

	rendr.Present();
	SDL_Delay(5000);
}

auto main (void) -> int 
{
	EMan world;
	sparseArray<int, 64000, 64> sparse;
	std::array<int, 64000> reg;
	for(int i = 0; i < 64000; i++)
	{
		sparse.insert(std::experimental::randint(0, 6399), 1);
		sparse.remove(std::experimental::randint(0, 6399));
	}

	auto start = std::chrono::high_resolution_clock::now();
	for(int i = 0; i < 10000; i++)
		sparse.print();

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << duration.count() << std::endl;
}



