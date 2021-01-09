#include <SDL2pp/Renderer.hh>
#include <iostream>
#include <exception>
#include <vector>
#include <unordered_set>
#include <string>
#include <experimental/random>
#include <map>

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

void benchmark (void)
{
	SparseArray<int, 64000> sparse;
	std::unordered_map<int, int> regular;
	std::vector<std::optional<int>> vec;

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
		for(int i = 0; i < 2500; i++) regular[std::experimental::randint(0, 63999)] = 1;
	};
	time(rArray);

	std::cout << "random access write, sparse array" << '\n';
	const auto rsArray = [&](){
		for(int i = 0; i < 2500; i++) sparse.insert(std::experimental::randint(0, 63999), 1);
	};
	time(rsArray);


	std::cout << "seq read/write, array" << '\n';
	const auto seqArray = [&](){
		for(int i = 0; i < 64000; i++)
		{
			if(regular[i])
				regular[i]++;
		}
	};
	time(seqArray);


	std::cout << "seq read/write, sparse" << '\n';
	const auto seqSparse = [&](){
		for(auto i : sparse) i++;
	};
	time(seqSparse);
}

struct pos 
{
	int x = 0;
	int y = 0;
};

struct color 
{
	using StorageStrategy = SparseArray<color>;

	int r = 255; 
	int g = 255; 
	int b = 255;
};

auto main (void) -> int 
{
	WorldSystems world;
	world.registerComponent<color>();
	world.registerComponent<pos>();

	auto ent = world.newEntity();
	world.addComponent<color>(ent, {255, 255, 255});
}





