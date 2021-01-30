#pragma once 
#include "star.h"
#include <SDL2pp/Renderer.hh>
#include <cmath>
#include <math.h>

#define PATH_MODE_LINEAR 0
#define PATH_MODE_BEZIER 1

using namespace SDL2pp;

//in an ECS than components are just raw data
//we can also put small helper functions in them as long as they're
//"free"

struct pos 
{
	float x = 0;
	float y = 0;
};

struct velocity
{
	int dx = 0;
	int dy = 0;

	double magnitude (void)
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

	velocity normalize (double scale = 1)
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

};

struct path
{
	int start_time = 0;
	int finish_time = 30000;
	int mode = PATH_MODE_LINEAR; 
	std::vector<pos> nodes = {};
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

struct collision
{
	int radius = 1;
};

//tag components too
struct renderTag
{};

struct animationTag
{};

struct playerTag
{};

struct pathTag
{};
