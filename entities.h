#include "components.h"
#include "systems.h"

Entity player (WorldSystems& world)
{
	auto e = world.newEntity();
	float xPos = std::experimental::randint(0, 640);
	float yPos = std::experimental::randint(0, 480);

	world.addComponent<renderTag>(e, {});
	world.addComponent<animationTag>(e, {});
	world.addComponent<pos>(e, {x: xPos, y: yPos});
	world.addComponent<playerTag>(e, {});
	world.addComponent<velocity>(e, {});

	return e;
}

Entity fireball (WorldSystems& world)
{
	Entity e = world.newEntity();

	world.addComponent<renderTag>(e, {});
	world.addComponent<animationTag>(e, {});
	world.addComponent<outOfBoundsTag>(e, {});
	world.addComponent<pos>(e, {std::experimental::randint(0, 640), -10});
	world.addComponent<velocity>(e, {0, std::experimental::randint(90, 150)});

	return e;
}

