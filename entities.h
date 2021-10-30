#include "components.h"
#include "systems.h"

Entity player (WorldSystems& world)
{
	auto e = world.newEntity();
	world.addComponent<renderTag>(e, {});
	world.addComponent<renderTag>(e, {});
	world.addComponent<animationTag>(e, {});
	world.addComponent<pos>(e, {x: std::experimental::randint(0, 640), y: std::experimental::randint(0, 480)});
	world.addComponent<velocity>(e, {0, 0});
	world.addComponent<playerTag>(e, {});
	world.addComponent<collision>(e, {100});
	world.addComponent<path>(e, {0, 7000, 1, {{25, 100}, {400, 500}, {200, 200}, {10, 10}, {480, 480}, {10, 160}, {160, 10}}});

	return e;
}


