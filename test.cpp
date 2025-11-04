#include "systems.h"
#include "entities.h"
#include "rtree.h"

void lalaGameSetup (WorldSystems& world, GameState& state, QTree&, Graphics&)
{
	player(world);
}


auto main (void) -> int
{
	Game game;
	//game.sceneBuild()
	auto sweep = game.addSystem(sweeper);
	game.addSystem(playerMove);
	game.addSystem(pathSystem);
	game.addSystem(animationSystem);
	game.addSystem(outOfBounds);
	game.addSystem(renderSystem)->before(sweep);
	game.addSystem(spawnFireBolts);
	game.addSystem(velocitySystem);
	game.addSystem(spaceSystem);
	game.addSystem(collision)->after(sweep);

	game.addSetup(lalaGameSetup);

	game.setup();

	while (true)
	{
		int code = game.gameLoop();
		if (code!= 0) break;
	}
}