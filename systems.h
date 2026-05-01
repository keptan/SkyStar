#pragma once 
#include "engine.h"
#include "components.h"
#include "star.h"
#include "paths.h"
#include "geometry.h"
#include "rtree.h"
#include <vector>
#include "entities.h"


/* we should make this generically rendered in the renderSyscastem
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
*/
/*
inline void playerMove (WorldSystems& world, GameState& state, QTree& space, Graphics& graphics)
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


		auto& lspace = world.getComponents<velocity>()->get(i);
		lspace = dir;
	}
}

inline void playerHexMove (WorldSystems& world, GameState& state, QTree& space, Graphics& graphics)
{
	auto sig = world.createSignature<hexPlayerTag, pos, hexTile>();
	auto ents = world.signatureScan(sig);




	for (const auto i : ents)
	{
		long int& cooldown = world.getComponents<hexPlayerTag>()->get(i).coolDown;


		const bool left =  ((state.input & InputMask::Left) == InputMask::Left);
		const bool right = ((state.input & InputMask::Right) == InputMask::Right);
		const bool up = ((state.input & InputMask::Up) == InputMask::Up);

		if (!left && !right && !up) cooldown = 0;

		if ((state.time - cooldown) < 500)
		{
			//std::print("skipping\n");
			continue;
		}

		int& frame = world.getComponents<hexTile>()->get(i).type.s.frame;
		auto& h = world.getComponents<hexTile>()->get(i);
		auto& p = world.getComponents<pos>()->get(i);

		if (left) frame++;
		if (right) frame--;
		if (frame > 7) frame = 0;
		if (frame < 0) frame = 7;

		if (up && !right && !left)
		{
			switch (frame)
			{
				case 0: h.hy++; break;
				case 1: h.hx++, h.hy++;break;
				case 2: h.hx++; break;
				case 3: h.hy--, h.hx++; break;
				case 4: h.hy--; break;
				case 5: h.hy--; h.hx--; break;
				case 6: h.hx--; break;
				case 7: h.hy++; h.hx--; break;

			}

			p = h.tSpace();
		}



		if (left || right || up) cooldown = state.time;
	}
}


inline void cameraMove (WorldSystems& world, GameState& state, QTree& space, Graphics& graphics)
{
	if ((state.input & InputMask::UArrow) == InputMask::UArrow) graphics.wy += (500 * state.frameTime) / 1000.0;
	if ((state.input & InputMask::DArrow) == InputMask::DArrow) graphics.wy -= (500 * state.frameTime) / 1000.0;
	if ((state.input & InputMask::LArrow) == InputMask::LArrow) graphics.wx += (500 * state.frameTime) / 1000.0;
	if ((state.input & InputMask::RArrow) == InputMask::RArrow) graphics.wx -= (500 * state.frameTime) / 1000.0;
	if ((state.input & InputMask::Home) == InputMask::Home)
	{
		graphics.wx = 0;
		graphics.wy = 0;
		graphics.ws = 1;
	}
	if ((state.input & InputMask::ScrollIn) == InputMask::ScrollIn) graphics.ws *= 1.015;
	if ((state.input & InputMask::ScrollOut) == InputMask::ScrollOut) graphics.ws /= 1.015;
	graphics.ws = std::clamp(graphics.ws, 0.1, 10.0);

}

/*
 *what we want to write
 *auto collect = world.createSignature<path, pos>();
 *auto space = collect.getComponents<pos>();
 *for (const auto archetype : space) for (const auto e )
 *space = archietype.getComponents<pos>()..
 */
/*
inline void pathSystem (WorldSystems& world, GameState& state, QTree&, Graphics&)
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
*/

/*
inline void hexRender (WorldSystems& world, GameState& state, QTree&, Graphics& graphics)
{
	auto sig = world.createSignature<pos, hexTile>();
	auto ents = world.signatureScan(sig);

	std::stable_sort(ents.begin(), ents.end(),
		[&](auto a, auto b){return world.getComponents<hexTile>()->get(a).hy < world.getComponents<hexTile>()->get(b).hy;});

	std::stable_sort(ents.begin(), ents.end(),
	[&](auto a, auto b){return world.getComponents<hexTile>()->get(a).hz < world.getComponents<hexTile>()->get(b).hz;});


	for(const auto i : ents)
	{
		auto& space = world.getComponents<pos>()->get(i);
		auto& tile = world.getComponents<hexTile>()->get(i);
		auto& texture = tile.type.s;

		graphics.rendr.Copy
		(
			*graphics.getTexture( texture.texture),
			SDL2pp::Rect(0, texture.height * texture.frame, texture.width, texture.height),
			SDL2pp::Rect((space.x + graphics.wx + tile.type.tx) * graphics.ws, (space.y + graphics.wy + tile.type.ty) * graphics.ws, texture.width * graphics.ws, texture.height * graphics.ws),
			space.rot * 180 / M_PI
		);
	}

}
*/

inline void renderSystem (World& world, GameState& state, QTree&, Graphics& graphics)
{

	const auto q = World::Query<pos, sprite, renderTag>(world);
	world.each(q, [&](const pos& space, const sprite& texture, const renderTag& r)
	{
		graphics.rendr.Copy
		(
			*graphics.getTexture( texture.texture),
			SDL2pp::Rect(0, texture.height * texture.frame, texture.width, texture.height),
			SDL2pp::Rect((space.x + graphics.wx) * graphics.ws, (space.y + graphics.wy) * graphics.ws, texture.width * graphics.ws, texture.height * graphics.ws),
			space.rot * 180 / M_PI
		);
	}
	);
}
/*
inline void animationSystem (WorldSystems& world, GameState& state, QTree& space, Graphics& graphics)
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
*/

inline void spawnFireBolts (World& world, GameState& state, QTree& space, Graphics& graphics)
{
	for (int i = 0; i < 100; i++) fireball(world, state);
}

inline void velocitySystem (World& world, GameState& state, QTree& space, Graphics& graphics)
{
	auto q = World::Query<pos, velocity>(world);
	world.each(q, [&](pos& p, const velocity& v)
	{
		p.x += v.dx * (state.frameTime / 1000.0);
		p.y += v.dy * (state.frameTime / 1000.0);
		p.rot += v.dr * (state.frameTime / 1000.0);
	});
}


inline void outOfBounds (World& world, GameState& state, QTree& space, Graphics& graphics)
{
/*

	for(const auto i: ents)
	{
		auto& p = world.getComponents<pos>()->get(i);
		if(p.y >= 500 || p.y <= -60 || p.x <= -20 || p.x >= 700) 
		world.killEntity(i);
	}
	*/
}
/*
 *
inline void collision (WorldSystems& world, GameState& state, QTree& space, Graphics& graphics)
{
	auto sig = world.createSignature<Rectangle, pos, outOfBoundsTag>();
	auto ents = world.signatureScan(sig);
	std::vector<Entity> killList;
	
	for(const auto i : ents)
	{
		auto& p = world.getComponents<pos>()->get(i);
		auto& r = world.getComponents<Rectangle>()->get(i);
		Rectangle translated{Point{p.x, p.y}, r.w, r.h};

		auto collisionEnts = space.query(translated);
		if(collisionEnts.size() > 1) killList.push_back(i);
	}

	for(const auto i : killList) world.killEntity(i);
}

inline void spaceSystem (WorldSystems& world, GameState& state, QTree& space, Graphics& graphics)
{
	auto sig 	= world.createSignature<Rectangle, pos>();
	auto ents = world.signatureScan(sig);
	space.qclear();

	graphics.rendr.SetDrawColor(0,0,255,255);

	for(const auto i : ents)
	{
		auto& p = world.getComponents<pos>()->get(i);
		auto& r = world.getComponents<Rectangle>()->get(i);
		Rectangle translated{Point{p.x, p.y}, r.w, r.h};
		graphics.rendr.DrawRect( SDL2pp::Rect( (translated.corner.x * 2), (translated.corner.y) * 2,  translated.w *2 , translated.h * 2));
		space.insert({translated, i});
	}
	space.balance();

	graphics.rendr.SetDrawColor(255,0,0,255);
	for(const auto& r : space.rectangles())
	{
		graphics.rendr.DrawRect( SDL2pp::Rect( (r.corner.x * 2), (r.corner.y) * 2, r.w * 2 ,  r.h * 2));
	}
}
*/
