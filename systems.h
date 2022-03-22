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
#include "box2d.h"
#include "b2_math.h"
#include "b2_world.h"
#include "b2_body.h"

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
	auto sig = world.createSignature<playerTag, pos>();
	auto ents = world.signatureScan(sig);

	for(const auto i : ents)
	{
		velocity dir{0, 0};
		if((state.input & InputMask::Up) == InputMask::Up) dir = dir + velocity{0, -50};
		if((state.input & InputMask::Left) == InputMask::Left) dir = dir + velocity{-50, 0};
		if((state.input & InputMask::Right) == InputMask::Right) dir = dir + velocity{50, 0};
		if((state.input & InputMask::Down) == InputMask::Down) dir = dir + velocity{0, 50};	

		dir = dir.normalize(150);
		if((state.input & InputMask::LShift) == InputMask::LShift) dir = dir.normalize(300);

		auto& space = world.getComponents<pos>()->get(i);
		auto& box	= world.getComponents<playerTag>()->get(i);

		auto p = box.body->GetPosition();
		auto a = box.body->GetAngle();
		box.body->SetLinearVelocity({dir.dx * 0.04, dir.dy * 0.04});

		space.x = p.x / 0.04;
		space.y = p.y / 0.04;
		space.rot = a;

		if(space.y < -32) box.body->SetTransform(b2Vec2(p.x, 488.0f * 0.04f), a);
		if(space.y > 488) box.body->SetTransform(b2Vec2(p.x, -16.0f * 0.04f), a);
		if(space.x > 640) box.body->SetTransform(b2Vec2(-8.0f * 0.04f,p.y), a);
		if(space.x < -8) box.body->SetTransform(b2Vec2(648.0f * 0.04f,p.y), a);
	
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

	//rendr.FillRect(0, 0, 640, 480);
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

void boxSystem (WorldSystems& world, GameState& state)
{
	auto sig = world.createSignature<pos, collision>();
	auto ents = world.signatureScan(sig);

	for(const auto i : ents)
	{
		auto& space = world.getComponents<pos>()->get(i);
		auto& box	= world.getComponents<collision>()->get(i);

		auto p = box.body->GetPosition();
		auto a = box.body->GetAngle();

		space.x = p.x / 0.04;
		space.y = p.y / 0.04;
		space.rot = a;

		if(space.y < -32) box.body->SetTransform(b2Vec2(p.x, 488.0f * 0.04f), a);
		if(space.y > 488) box.body->SetTransform(b2Vec2(p.x, -16.0f * 0.04f), a);
		if(space.x > 640) box.body->SetTransform(b2Vec2(-8.0f * 0.04f,p.y), a);
		if(space.x < -8) box.body->SetTransform(b2Vec2(648.0f * 0.04f,p.y), a);
	}

}

void naiveMissileControl (WorldSystems& world, GameState& state)
{
	auto sig = world.createSignature<missile, collision>();
	auto ents = world.signatureScan(sig);

	for(const auto i : ents)
	{
		auto& box = world.getComponents<collision>()->get(i);
		auto& rocket = world.getComponents<missile>()->get(i);
		auto& target	= world.getComponents<playerTag>()->get(rocket.target);

		auto posA  = box.body->GetPosition();
		auto posB  = target.body->GetPosition();

		const int sign = box.body->GetAngle() > 0 ? 1 : -1;
		auto propAngle	= sign * fmod(abs(box.body->GetAngle() *  180 /M_PI), 360);
		auto spaceAngle	= atan2(posA.x - posB.x, posA.y - posB.y) * 180 / M_PI;
		if(spaceAngle <0) spaceAngle = 360 - abs(spaceAngle) ;

		auto error = [&]()
		{
			auto angleDiff = propAngle - spaceAngle;
			const int sign = angleDiff > 0 ? 1 : -1;

			std::cout << angleDiff << ' ' << propAngle << ' ' << spaceAngle << std::endl;

			//if(angleDiff > 180) return angleDiff - 360;
			//if(angleDiff < -180) return angleDiff + 360;

			return angleDiff;
		}();

		auto correction = [&]()
		{
			const int sign = error > 0 ? 1 : -1;
			const auto mag = abs(error);
			if(mag > 5) return sign * 3.5;
			return sign * ((mag / 5) * (mag/ 5));
		}();

		//std::cout << propAngle << ' ' << spaceAngle << ' ' << error << ' ' << correction << std::endl;

		rocket.controller.setTarget(correction);
		auto adjust		= rocket.controller.tick(box.body->GetAngularVelocity(), state.time);
		box.body->ApplyTorque(adjust, true);
		auto unadjusted	= box.body->GetAngle() - 1.5708;



		auto x = cos(unadjusted);
		auto y = sin(unadjusted);
		b2Vec2 vec = b2Vec2(x, y);
		vec *= 0.33;
		//box.body->ApplyForceToCenter(vec, true);
	}
}


