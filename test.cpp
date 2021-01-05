#include <SDL2pp/Renderer.hh>
#include <iostream>
#include <exception>
#include <vector>
#include <unordered_set>
#include <string>
#include <experimental/random>
#include <unordered_map>

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
	sparseArray<int, 6400, 64> sparse;
	std::unordered_map<int, int> regular;

	const auto time = [&](const auto f)
	{
		auto start = std::chrono::high_resolution_clock::now();
		f();
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
		std::cout << "function took: " << duration.count() << " milliseconds \n";
	};
	std::cout << "random access write, array" << '\n';
	const auto rArray = [&](){
		for(int i = 0; i < 10000; i++) regular[std::experimental::randint(0, 6399)] = 1;
	};
	time(rArray);

	std::cout << "random access write, sparse array" << '\n';
	const auto rsArray = [&](){
		for(int i = 0; i < 10000; i++) sparse.insert(std::experimental::randint(0, 6399), 1);
	};
	time(rsArray);

	std::cout << "seq read/write, array" << '\n';
	const auto seqArray = [&](){
		for(auto i : regular) i.second++;
	};
	time(seqArray);


	std::cout << "seq read/write, sparse" << '\n';
	const auto seqSparse = [&](){
		for(auto i : sparse) i++;
	};
	time(seqSparse);

}





