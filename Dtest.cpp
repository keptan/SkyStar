//
// Created by mimo on 3/15/26.
//



#include "star.h"
#include <print>

struct pos
{
	int x = 0;
	int y = 0;
};

struct vel
{
	int x = 0;
};

struct bounce
{
	int b = 10;
};

int main (void)
{
	World world;
	const auto s = createSignature<pos, vel>();
	auto e =0;

	for (int i = 0; i < 2000005; i++)
	{
		world.emplaceEntity(pos{i,i}, vel{0});
	}

	for (int i = 25000; i < 2500001; i++)
	{
		world.emplaceEntity(pos{i, i}, bounce{1}, vel {1});
	}

	//std::print("{}\n", v[0].x);

	auto q = World::Query<pos, vel>(world);

	world.each(q, [](const pos& p, const vel& v){std::print("{}\n", p.x);});
	return 0;
}