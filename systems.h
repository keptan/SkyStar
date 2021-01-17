#pragma once 
#include "engine.h"
#include "components.h"
#include <SDL2pp/Renderer.hh>
#include <random>
//systems are functions that operate on a subset of entities 

using namespace SDL2pp;
void moveSystem (WorldSystems& world, GameState& state)
{
	auto sig = world.createSignature<pos, velocity>();
	auto ents = world.signatureScan(sig);

	auto space = world.getComponents<pos>();
	auto vel   = world.getComponents<velocity>();

	for(const auto i : ents)
	{
		auto& s  = space->get(i);
		auto&  dir   = vel->get(i);

		s.x += (double(state.frameTime) / 1000) * (double (dir.dx));
		s.y += (double(state.frameTime) / 1000) * (double (dir.dy));
		if(s.x > 640) s.x = -32;
		if(s.y > 480)
		{
			s.x = std::experimental::randint(0, 640);
			s.y = -32;
		}
		if(s.x < -32) s.x = 640;
		if(s.y < -32) s.y = 480;

	}
}

void renderWall (WorldSystems& world, GameState& state, Renderer& rendr, std::shared_ptr<Texture> t)
{
	const int offset = state.frameCount % 64;
	for(int x = -64; x < 640; x += 64)
	{
		for(int y = -64; y < 480; y+= 64)
		{
			rendr.Copy(*t, Rect(0, 0, 64, 64), Rect(x, y + offset, 64, 64));
		}
	}
}

void playerMove (WorldSystems& world, GameState& state)
{
	auto sig = world.createSignature<playerTag, velocity>();
	auto ents = world.signatureScan(sig);
	auto vel   = world.getComponents<velocity>();

	for(const auto i : ents)
	{
		auto& v = vel->get(i);
		velocity dir{0, 0};
		if((state.input & InputMask::Up) == InputMask::Up) dir = dir + velocity{0, -50};
		if((state.input & InputMask::Left) == InputMask::Left) dir = dir + velocity{-50, 0};
		if((state.input & InputMask::Right) == InputMask::Right) dir = dir + velocity{50, 0};
		if((state.input & InputMask::Down) == InputMask::Down) dir = dir + velocity{0, 50};	

		dir = dir.normalize(150);

		v = dir;
	}
	auto space = world.getComponents<pos>();
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
		if(state.frameCount % 7 == 0)
		{
			texture.frame++; 
			if(texture.frame > texture.frames) texture.frame = 0;
		}

	}
}
