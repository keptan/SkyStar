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

Entity fireball (WorldSystems& world, GameState& state)
{
	Entity e = world.newEntity();

	auto callback = [](Entity e, WorldSystems& w, GameState& s)
	{
		w.killEntity(e);
	};

	world.addComponent<renderTag>(e, {});
	world.addComponent<animationTag>(e, {});
	world.addComponent<outOfBoundsTag>(e, {});
	world.addComponent<pos>(e, {std::experimental::randint(0, 640), -10});
//	world.addComponent<velocity>(e, {0, std::experimental::randint(90, 150)});
	world.addComponent<path>(e, {.start_time = state.time, .finish_time = state.time + 5000, .nodes = generateRandomNodes()});
	world.addComponent<pCallback>(e, pCallback(callback, 5000, state.time));

	return e;
};


