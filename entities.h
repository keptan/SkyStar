#include "components.h"
#include "systems.h"

Entity player (WorldSystems& world, b2World& space)
{
	auto e = world.newEntity();
	b2BodyDef player;

	float xPos = std::experimental::randint(0, 640);
	float yPos = std::experimental::randint(0, 480);

	player.type = b2_kinematicBody;
	player.position.Set(xPos * 0.04f, yPos * 0.04f);

	auto* body = space.CreateBody(&player);
	b2PolygonShape box;
	box.SetAsBox(21.0f * 0.02f, 37.0f * 0.02f);
	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.friction = 0.3f;

	body->CreateFixture(&fixture);

	world.addComponent<renderTag>(e, {});
	world.addComponent<renderTag>(e, {});
	world.addComponent<animationTag>(e, {});
	world.addComponent<pos>(e, {x: xPos, y: yPos});
	world.addComponent<playerTag>(e, {body});
	//world.addComponent<collision>(e, {{width: 21, height: 37}});
	//one pixel = 4cm
	//there, I said it!

	return e;
}



Entity fireball (WorldSystems& world, b2World& space, const Entity target)
{
	Entity e = world.newEntity();
	b2BodyDef fireball;

	float xPos = std::experimental::randint(0, 640);
	float yPos = std::experimental::randint(0, 480);
//	float xVel = std::experimental::randint(-15, 15);
//	float yVel = std::experimental::randint(50, 140);
	float xVel = 0;
	float yVel = 0;

	fireball.type = b2_dynamicBody;
	fireball.position.Set(xPos * 0.04f, yPos * 0.04f);
	fireball.linearVelocity.Set(xVel * 0.04f, yVel * 0.04f);

	auto* body = space.CreateBody(&fireball);

	b2PolygonShape box;
	box.SetAsBox(8.0f * 0.02f, 16.0f * 0.02f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;

	body->CreateFixture(&fixtureDef);

	world.addComponent<renderTag>(e, {});
	world.addComponent<animationTag>(e, {});
	world.addComponent<pos>(e, {std::experimental::randint(0, 640), std::experimental::randint(0, 480)});
	world.addComponent<collision>(e, {body});
	world.addComponent<missile>(e, target);


	//world.addComponent<collision>(e, {{width: 8, height: 16}});
    return e;
}

Entity block (WorldSystems& world, b2World& space)
{
	Entity e = world.newEntity();
	b2BodyDef block;
	float xPos = std::experimental::randint(0, 640);
	float yPos = std::experimental::randint(0, 480);
	float size = 10;

	block.type = b2_staticBody;
	block.position.Set(xPos * 0.04f, yPos * 0.04f);

	auto* body = space.CreateBody(&body);
	b2PolygonShape box;
	box.SetAsBox(10.0f * 0.02f, 10.0f * 0.02f);
}

}
