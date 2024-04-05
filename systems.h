#pragma once 
#include "engine.h"
#include "components.h"
#include "star.h"
#include "paths.h"
#include "geometry.h"
#include <SDL2pp/Renderer.hh>
#include <vector>
#include <random>
#include <algorithm>
#include <span>

//systems are functions that operate on a subset of entities 

void renderWall (WorldSystems& world, GameState& state, SDL2pp::Renderer& rendr, std::shared_ptr<SDL2pp::Texture> t)
{
	const int offset = state.frameCount % 64;
	for(int x = (-64 * 2); x < 640 * 2; x += 64 * 2)
	{
		for(int y =( -64 * 2); y < 480 * 2; y+= 64 * 2)
		{
			rendr.Copy(*t, SDL2pp::Rect(0, 0, 64, 64), SDL2pp::Rect(x, y + offset * 2, 64 * 2, 64 * 2));
		}
	}
}

void playerMove (WorldSystems& world, GameState& state)
{
	auto sig = world.createSignature<playerTag, pos, velocity>();
	auto ents = world.signatureScan(sig);

	for(const auto i : ents)
	{
		velocity dir{0, 0};
		if((state.input & InputMask::Up) == InputMask::Up) dir = dir + velocity{0, -50};
		if((state.input & InputMask::Left) == InputMask::Left) dir = dir + velocity{-50, 0};
		if((state.input & InputMask::Right) == InputMask::Right) dir = dir + velocity{50, 0};
		if((state.input & InputMask::Down) == InputMask::Down) dir = dir + velocity{0, 50};	

		if((state.input & InputMask::LShift) == InputMask::LShift) dir = dir.normalize(500);
			else dir = dir.normalize(250);


		auto& space = world.getComponents<velocity>()->get(i);
		space = dir;
	}
}

void pathSystem (WorldSystems& world, GameState& state)
{
	auto sig = world.createSignature<path>();
	auto ents = world.signatureScan(sig);
	auto space = world.getComponents<pos>();
	auto paths = world.getComponents<path>();

	for(const auto i : ents)
	{
		auto& s = space->get(i);
		auto& p = paths->get(i);

		if ( state.time < p.start_time ) continue;

		if      ( p.mode == PATH_MODE_LINEAR ) 
		{
			s = linearPath( state.time - p.start_time, p.finish_time, p.nodes );
		} 
		else if ( p.mode == PATH_MODE_BEZIER ) 
		{
			s = bezierPath( state.time - p.start_time, p.finish_time, p.nodes );
		}
	}
}

void renderSystem (WorldSystems& world, GameState& state, SDL2pp::Renderer& rendr)
{
	auto sig = world.createSignature<pos, sprite, renderTag>();
	auto ents = world.signatureScan(sig);

	for(const auto i : ents)
	{
		auto& space = world.getComponents<pos>()->get(i);
		auto& texture = world.getComponents<sprite>()->get(i);

		rendr.Copy
		(
			*texture.sheet, 
			SDL2pp::Rect(0, texture.height * texture.frame, texture.width, texture.height), 
			SDL2pp::Rect(space.x * 2, space.y * 2, texture.width * 2, texture.height * 2),
			space.rot * 180 / M_PI	
		);
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

void velocitySystem (WorldSystems& world, GameState& state)
{
	auto sig  = world.createSignature<velocity, pos>();
	auto ents = world.signatureScan(sig);

	for(const auto i: ents)
	{
		auto& p = world.getComponents<pos>()->get(i);
		auto& v = world.getComponents<velocity>()->get(i);
		p.x += v.dx * (state.frameTime / 1000.0);
		p.y += v.dy * (state.frameTime / 1000.0);
	}
}

