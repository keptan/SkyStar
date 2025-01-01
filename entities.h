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
	world.addComponent<Rectangle>(e, {{0,0}, 10, 10});

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
	world.addComponent<velocity>(e, {0, std::experimental::randint(90, 150)});
	world.addComponent<Rectangle>(e, {{0,0}, 10, 10});


	return e;
};

Entity bolt (WorldSystems& world, GameState& state)
{
	auto sig  = world.createSignature<playerTag, pos>();
	auto ents = world.signatureScan(sig);
	Entity e  = world.newEntity();

	for(const auto i : ents)
	{
		auto space = world.getComponents<pos>()->get(i);

		world.addComponent<renderTag>(e, {});
		world.addComponent<animationTag>(e, {});
		world.addComponent<outOfBoundsTag>(e, {});

		world.addComponent<pos>(e, space);
		world.addComponent<velocity>(e, {0, -250});
		world.addComponent<Rectangle>(e, {{0,0}, 10, 10});
	}

	return e;
};


