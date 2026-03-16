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
	const auto e = world.createEntity(s);

	const auto v = world.archetypes.at(s.archetypeID).getColumn<pos>(world.entities.getRecordPointer(e)->rowIndex);
	v[0].x = 10;

	std::print("{}\n", v[0].x);


	return 0;
}