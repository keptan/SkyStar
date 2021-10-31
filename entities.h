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
	//world.addComponent<collision>(e, {{width: 21, height: 37}});
	//one pixel = 4cm
	//there, I said it!
//	world.addComponent<path>(e, {0, 7000, 1, {{25, 100}, {400, 500}, {200, 200}, {10, 10}, {480, 480}, {10, 160}, {160, 10}}});

	return e;
}



Entity fireball (WorldSystems& world, b2World& space)
{
	Entity e = world.newEntity();
	b2BodyDef fireball;

	float xPos = std::experimental::randint(0, 640);
	float yPos = std::experimental::randint(0, 480);
	float xVel = std::experimental::randint(-15, 15);
	float yVel = std::experimental::randint(50, 140);

	fireball.type = b2_dynamicBody;
	fireball.position.Set(xPos * 0.04f, yPos * 0.04f);
	fireball.linearVelocity.Set(xVel * 0.04f, yVel * 0.04f);

	auto* body = space.CreateBody(&fireball);

	b2PolygonShape box;
	box.SetAsBox(8.0f * 0.04f, 16.0f * 0.04f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;

	body->CreateFixture(&fixtureDef);

	world.addComponent<renderTag>(e, {});
	world.addComponent<animationTag>(e, {});
	world.addComponent<pos>(e, {std::experimental::randint(0, 640), std::experimental::randint(0, 480)});
	world.addComponent<collision>(e, {body});


	//world.addComponent<collision>(e, {{width: 8, height: 16}});
    return e;
}
