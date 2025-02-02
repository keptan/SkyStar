#pragma once 
#include "engine.h"
#include "components.h"
#include "star.h"
#include "paths.h"
#include "geometry.h"
#include "rtree.h"
#include <vector>
#include <random>
#include <algorithm>

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

void outOfBounds (WorldSystems& world, GameState& state)
{
	auto sig  = world.createSignature<outOfBoundsTag, pos>();
	auto ents = world.signatureScan(sig);

	for(const auto i: ents)
	{
		auto& p = world.getComponents<pos>()->get(i);
		if(p.y >= 500 || p.y <= -60 || p.x <= -20 || p.x >= 700) 
		world.killEntity(i);
	}
}

void spaceSystem (WorldSystems& world, GameState& state, SDL2pp::Renderer& rendr, QTree& space)
{
	auto sig 	= world.createSignature<Rectangle, pos>();
	auto ents = world.signatureScan(sig);
	space.eclear();
	space.qclear();

	for(const auto i : ents)
	{
		auto& p = world.getComponents<pos>()->get(i);
		auto& r = world.getComponents<Rectangle>()->get(i);
		Rectangle translated{Point{p.x, p.y}, r.w, r.h};
		space.insert({translated, i});
	}
	space.balance();

	for(const auto& r : space.rectangles())
	{
		rendr.SetDrawColor(255,0,0,255);
		rendr.DrawRect( SDL2pp::Rect( (r.corner.x * 2), (r.corner.y) * 2, (r.corner.x + r.w) * 2, (r.corner.y + r.h) * 2));
	}

}

void pCallbackSystem (WorldSystems& world, GameState& state)
{
	auto sig  = world.createSignature<pCallback>();
	auto ents = world.signatureScan(sig);

	for(const auto i : ents)
	{
		auto& p = world.getComponents<pCallback>()->get(i);
		if(state.time >= p.time) 
		{
			p.time = state.time + p.period;
			p.f_p(i, world, state); 

		}
	}
}

