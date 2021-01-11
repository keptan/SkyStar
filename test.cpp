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
#include <random>
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


template <typename F>
unsigned int time (const F f)
{
	auto start = std::chrono::high_resolution_clock::now();
	f();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
	return duration.count();
};

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
	float x = 0;
	float y = 0;
};

struct velocity
{
	float vt;
	float angle;

};

struct sprite 
{
	using StorageStrategy = SparseArray<sprite>;
	std::shared_ptr<Texture> sheet;
	int size = 32;
};

struct renderTag
{};

void moveSystem (WorldSystems& world, unsigned int dt)
{
	auto sig = world.createSignature<pos, velocity>();
	auto ents = world.signatureScan(sig);

	auto space = world.getComponents<pos>();
	auto vel   = world.getComponents<velocity>();

	for(const auto i : ents)
	{
		auto& s  = space->get(i);
		auto&  v   = vel->get(i);

		s.y += (float(dt) / 1000) * -v.vt * std::cos(v.angle);
		s.x += (float(dt) / 1000) * v.vt * std::sin(v.angle);
		if(s.x > 640) s.x = -32;
		if(s.y > 480) s.y = -32;
		if(s.x < -32) s.x = 640;
		if(s.y < -32) s.y = 480;

		v.angle += 0.01;
	}
}

void renderSystem (WorldSystems& world, unsigned int dt, Renderer& rendr)
{
	auto sig = world.createSignature<pos, sprite, renderTag>();
	auto ents = world.signatureScan(sig);

	rendr.FillRect(0, 0, 640, 480);
	for(const auto i : ents)
	{
		auto& space = world.getComponents<pos>()->get(i);
		auto texture = world.getComponents<sprite>()->get(i);

		rendr.Copy(*texture.sheet, Rect(0, 0, texture.size, texture.size), Rect(space.x, space.y, 32, 32));
	
	}
	rendr.Present();
}


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
	world.registerComponent<velocity>();

	for(int i = 0; i < 60; i++)
	{
		auto e = world.newEntity();
		world.addComponent<renderTag>(i, {});
		world.addComponent<sprite>(i, {std::make_shared<Texture>(rendr, DATA_PATH "/lala.png"), 32});
		world.addComponent<pos>(i, {std::experimental::randint(0, 640), std::experimental::randint(0, 480)});
		world.addComponent<velocity>(i, {std::experimental::randint(50, 300), std::experimental::randint(0, 0)});
	}


	auto frames = SDL_GetTicks();
	auto lastFrame = 0;

	for(int i = 0; i < 1000; i++)
	{
	auto times = SDL_GetTicks();
	auto dt = times - frames;
	frames = times;
	lastFrame = dt;
	moveSystem(world, dt);
	renderSystem(world, dt, rendr);
	SDL_Delay(16 < dt ? 16 : 16 - dt );
	}

}





