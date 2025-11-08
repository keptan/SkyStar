#include "systems.h"
#include "entities.h"
#include "rtree.h"
#include "debugGraphics.h"

#include <functional>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

void lalaGameSetup (WorldSystems& world, GameState& state, QTree&, Graphics&)
{
	player(world);
}


auto main (void) -> int
{
	Game game;

	game.addSystem(sweeper);
	game.addSystem(playerMove);
	game.addSystem(pathSystem);
	game.addSystem(animationSystem);
	game.addSystem(outOfBounds);
	auto rendr = game.addSystem(renderSystem);
	game.addSystem(spawnFireBolts);
	game.addSystem(velocitySystem);
	auto space = game.addSystem(spaceSystem);
	auto col = game.addSystem(collision);
	game.addSystem(graphicsRun)->after(rendr).after(col).after(space);

	game.addSetup(lalaGameSetup);
	game.addSetup(graphicsSetup);
	game.setup();
	while (true)
	{
		int code = game.gameLoop();
		if (code!= 0) break;
	}

	debugGraphicsClose();
}