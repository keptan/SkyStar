#pragma once 
#include "star.h"
#include "geometry.h"
#include "engine.h"
#include <cmath>


#define PATH_MODE_LINEAR 0
#define PATH_MODE_BEZIER 1


//in an ECS than components are just raw data
//but here we go putting functions into them

struct velocity
{
	double dx = 0;
	double dy = 0;

	double magnitude (void) const
	{
		//a^2 + b^2 = c^2
		return std::sqrt( std::pow(dx, 2) +  std::pow(dy, 2));
	}

	velocity operator+ (const velocity& rhs) const
	{
		velocity out;
		out.dx = dx + rhs.dx;
		out.dy = dy + rhs.dy;
		return out;
	}

	velocity normalize (double scale = 1) const
	{
		velocity out;
		const auto m = magnitude();
		if(m > 0)
		{
			out.dx = (dx / m) * scale;
			out.dy = (dy / m) * scale;
		}
		return out;
	}

	struct args
	{
		const double x = 0.0;
		const double y = 0.0;
	};

	 velocity (const args a)
		: dx(a.x), dy(a.y)
	{}

	velocity (const double x = 0.0, const double y = 0.0)
		: dx(x), dy(y)
	{}
};

struct path
{
	long int start_time = 0;
	long int finish_time = 30000;
	int mode = PATH_MODE_LINEAR; 
	std::vector<pos> nodes = {};

};
	
struct sprite 
{
	using StorageStrategy = SparseArray<sprite>;
	std::string texture;
	int height = 32;
	int width  = 32;
	int frames = 0; //this should be moved to a different component
					//so that sprite is just a ptr to the sprite data
					//itself I guess
					//are spritesheets fundamental or just an accessory feature?
					//what are the costs?
	int frame  = 0;
};

//tag components too
struct renderTag
{};

struct animationTag
{};

struct playerTag
{};

struct outOfBoundsTag
{};

struct sdlRect
{
	Rectangle r;
};

struct gravityTag{};
struct event
{
	long int time;
};

