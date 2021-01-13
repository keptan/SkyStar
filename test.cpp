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


template <typename F>
unsigned int time (const F f)
{
	auto start = std::chrono::high_resolution_clock::now();
	f();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
	return duration.count();
};

struct GameState
{
	unsigned int frameCount = 0;
	unsigned int time = 0;
	unsigned int frameTime = 0;
};


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
	int height = 32;
	int width  = 32;
	int frames = 0;
	int frame  = 0;
};

struct renderTag
{};

struct animationTag
{};

void moveSystem (WorldSystems& world, GameState& state)
{
	auto sig = world.createSignature<pos, velocity>();
	auto ents = world.signatureScan(sig);

	auto space = world.getComponents<pos>();
	auto vel   = world.getComponents<velocity>();

	for(const auto i : ents)
	{
		auto& s  = space->get(i);
		auto&  v   = vel->get(i);

		s.y += (float(state.frameTime) / 1000) * -v.vt * std::cos(v.angle);
		s.x += (float(state.frameTime) / 1000) * v.vt * std::sin(v.angle);
		if(s.x > 640) s.x = -32;
		if(s.y > 480) s.y = -32;
		if(s.x < -32) s.x = 640;
		if(s.y < -32) s.y = 480;

		v.angle += 0.01;
	}
}

void renderWall (Renderer& rendr, std::shared_ptr<Texture> t)
{
	for(int x = 0; x < 640; x += 64)
	{
		for(int y = 0; y < 480; y+= 64)
		{
			rendr.Copy(*t, Rect(0, 0, 64, 64), Rect(x, y, 64, 64));
		}
	}
}


void renderSystem (WorldSystems& world, GameState& state, Renderer& rendr)
{
	auto sig = world.createSignature<pos, sprite, renderTag>();
	auto ents = world.signatureScan(sig);

	//rendr.FillRect(0, 0, 640, 480);
	for(const auto i : ents)
	{
		auto& space = world.getComponents<pos>()->get(i);
		auto& texture = world.getComponents<sprite>()->get(i);

		rendr.Copy(*texture.sheet, Rect(0, texture.height * texture.frame, texture.width, texture.height), Rect(space.x, space.y, texture.width, texture.height));
	
	}
	rendr.Present();
}

void animationSystem (WorldSystems& world, GameState& state)
{
	auto sig = world.createSignature<sprite, animationTag>();
	auto ents = world.signatureScan(sig);
	for(const auto i : ents)
	{
		auto& texture = world.getComponents<sprite>()->get(i);
		if(!state.frameCount % 30 )
		{
			texture.frame = texture.frame + 1;

			if(texture.frame > texture.frames) texture.frame = 0;
		}
	}
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
	world.registerComponent<animationTag>();
	world.registerComponent<sprite>();
	world.registerComponent<velocity>();

	auto player = std::make_shared<Texture>(rendr, DATA_PATH "/lala.png");
	auto fire	= std::make_shared<Texture>(rendr, DATA_PATH "/flame.png");
	auto wallpaper = std::make_shared<Texture>(rendr, DATA_PATH "/wall.png");

	for(int i = 0; i < 60; i++)
	{
		auto e = world.newEntity();
		world.addComponent<renderTag>(i, {});
		world.addComponent<animationTag>(i, {});
		world.addComponent<sprite>(i, {fire, 16, 8, 1, 1});
		world.addComponent<pos>(i, {std::experimental::randint(0, 640), std::experimental::randint(0, 480)});
		world.addComponent<velocity>(i, {std::experimental::randint(50, 300), std::experimental::randint(0, 0)});
	}


	GameState state; 
	state.time = SDL_GetTicks();

	for(int i = 0; i < 1000; i++)
	{
	auto times = SDL_GetTicks();
	state.frameTime = times - state.time;
	state.time = times;
	moveSystem(world, state);
	animationSystem(world, state);
	renderWall(rendr, wallpaper);
	renderSystem(world, state, rendr);

	state.frameCount++;
	SDL_Delay(16 < state.frameTime ? 16 : 16 - state.frameTime);
	}

}





