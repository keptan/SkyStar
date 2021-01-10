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

struct sprite 
{
	using StorageStrategy = SparseArray<sprite>;
	std::shared_ptr<Texture> sheet;
	int size = 32;
};

struct renderTag
{};

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
	world.registerComponent<sprite>();

	for(int i = 0; i < 50; i++)
	{
		auto e = world.newEntity();
		world.addComponent<renderTag>(i, {});
		world.addComponent<sprite>(i, {std::make_shared<Texture>(rendr, DATA_PATH "/lala.png"), 32});
		world.addComponent<pos>(i, {std::experimental::randint(0, 640), std::experimental::randint(0, 480)});
	}


	auto sig = world.createSignature<sprite, renderTag, pos>();
	auto ents = world.signatureScan(sig);

	const auto blit = [&](){
	for(const auto i : ents)
	{
		auto space = world.getComponents<pos>()->get(i);
		auto texture = world.getComponents<sprite>()->get(i);

		rendr.Copy(*texture.sheet, Rect(0, 0, texture.size, texture.size), Rect(space.x, space.y, 32, 32));
	};};

	const auto time = [&](const auto f)
	{
		auto start = std::chrono::high_resolution_clock::now();
		f();
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
		std::cout << "function took: " << duration.count() << " milliseconds \n";
	};

	time(blit);
	rendr.Present();
	SDL_Delay(5000);
}





