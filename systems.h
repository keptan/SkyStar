#pragma once 
#include "engine.h"
#include "components.h"
#include "star.h"
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





	

struct SpaceGrid 
{
	std::vector< std::vector<Entity>> res;
	const int fidelity;
	const int width; 
	const int height;

	SpaceGrid (WorldSystems& world, const int fidelity, const int width, const int height) 
		: fidelity(fidelity), width(width), height(height)
	{
		const int boxes =  ((width +  1 / fidelity) + 1)  * ((height  / fidelity) + 1);
		res.reserve(boxes);
		for(int i = 0; i < boxes; i++) res.push_back({});
	}

	void regen (WorldSystems& world) 
	{
		auto sig  = world.createSignature<pos, collision>();
		auto ents = world.signatureScan(sig);

		for(auto& v : res)
		{
			v.clear();
		}

		
		for(const auto i : ents)
		{
			const auto& space = world.getComponents<pos>()->get(i);	
			const auto& bound = world.getComponents<collision>()->get(i);	

			if(space.x < 0 || space.y < 0) continue;
			if(space.x > 640 || space.y > 480) continue;

			const int place = ((int(space.y) / fidelity) * (width / fidelity)) + (int(space.x) / fidelity);
			res.at(place).push_back(i);
		}
	}

	const std::vector<Entity>& adjacent (const collision& c, const pos& space) const
	{

		const int place = (( int(space.y) / fidelity) * (width / fidelity)) + (int(space.x) / fidelity);
		return res.at(place);
	
	}
};

void collisionSphere (WorldSystems& world, GameState& state, SpaceGrid& space, std::shared_ptr<Texture> t, std::shared_ptr<Texture> normal)
{

	auto sig = world.createSignature<sprite, pos, collision>();
	auto ents = world.signatureScan(sig);

	auto player = world.signatureScan( world.createSignature<playerTag, pos, collision>());
	for(const auto p : player)
	{
		for(const auto e : ents)
		{
			if(e == p) continue;

			auto& s = world.getComponents<sprite>()->get(e);

			const auto position   = world.getComponents<pos>()->get(e);
			if(position.x < 0 || position.y < 0 ) continue;

			const auto col    = world.getComponents<collision>()->get(e);
			const auto& adjacent = space.adjacent(col, position);

			if(std::find(adjacent.begin(), adjacent.end(), p) != adjacent.end())
			{
				s.sheet = t;
			}
			else 
			{
				s.sheet = normal;
			}
		}
	}
}
