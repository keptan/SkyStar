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

int main (void)
{
	World world;
	const auto s = createSignature<pos, vel>();
	auto e =0;

	for (int i = 0; i < 25000; i++)
	{
		e = world.emplaceEntity(pos{i,i}, vel{0});
	}

	//std::print("{}\n", v[0].x);

	auto q = World::Query<pos, vel>(world);

	world.each(q, [](const pos& p, const vel& v){std::print("{}\n", p.x);});

	return 0;
}