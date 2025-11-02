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
	game.addSystem(sweeper);
	game.addSystem(playerMove);
	game.addSystem(pathSystem);
	game.addSystem(animationSystem);
	game.addSystem(outOfBounds);
	game.addSystem(renderSystem);
	game.addSystem(spawnFireBolts);
	game.addSystem(velocitySystem);
	game.addSystem(spaceSystem);
	game.addSystem(collision);

	game.addSetup(lalaGameSetup);

	game.setup();

	while (true)
	{
		int code = game.gameLoop();
		if (code!= 0) break;
	}
}/*
		if ((state.input & InputMask::PKey) == InputMask::PKey)
		{
			for(int i = 0; i < 25; i++)
			{
				auto e = fireball(world, state);
				world.addComponent<sprite>(e, {fire, 16, 8, 1, 0});
				std::cout << world.eCount() << std::endl;
			}
		}
		if ((state.input & InputMask::Space) == InputMask::Space)
		{
			if(state.frameCount % 10 == 0)
			{
				auto e = bolt(world, state);
				world.addComponent<sprite>(e, {lBolt, 16, 8, 1, 0});
			}
		}
		*/

		/*
		sweeper(world, state);
		playerMove(world, state);
		pathSystem(world, state);
		animationSystem(world, state);
		renderWall(world, state, window.rendr, wallpaper);
		renderSystem(world, state, window.rendr);
		velocitySystem(world, state);
		outOfBounds(world, state);
		spaceSystem(world, state, window.rendr, space);
		collision(world, state, space);

		window.rendr.Present();

		state.frameCount++;
		const auto endFrame = SDL_GetTicks();
		const auto frameCost = endFrame - tick;
		averageFrameTime+= frameCost;
		SDL_Delay(frameCost >= 7 ? 0 : 7 - frameCost);
		*/

	//std::cout << averageFrameTime / state.frameCount << '\n';
