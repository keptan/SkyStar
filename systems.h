#pragma once 
#include "engine.h"
#include "components.h"
#include <SDL2pp/Renderer.hh>
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
		auto&  v   = vel->get(i);
		v.angle = 0;
		if((state.input & InputMask::Up) == InputMask::Up) v.angle += 0;
		if((state.input & InputMask::Down) == InputMask::Down) v.angle = 0;
		if((state.input & InputMask::Up) == InputMask::Up) v.angle = 0;

		s.y += (float(state.frameTime) / 1000) * -v.vt * std::cos(v.angle);
		s.x += (float(state.frameTime) / 1000) * v.vt * std::sin(v.angle);
		if(s.x > 640) s.x = -32;
		if(s.y > 480) s.y = -32;
		if(s.x < -32) s.x = 640;
		if(s.y < -32) s.y = 480;

		v.angle += 0.01 * std::experimental::randint(-10, 10);
	}
}

void renderWall (WorldSystems& world, GameState& state, Renderer& rendr, std::shared_ptr<Texture> t)
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
		if(state.frameCount % 20 == 0)
		{
			texture.frame++; 
			if(texture.frame > texture.frames) texture.frame = 0;
		}

	}
}
