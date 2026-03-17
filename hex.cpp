#include "systems.h"
#include "entities.h"
#include "rtree.h"
#include "debugGraphics.h"
#include "components.h"

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>


void hexGameSetup (WorldSystems& world, GameState& state, QTree&, Graphics&)
{
	return;
}

void animeSpin (WorldSystems& world, GameState& state, QTree& q, Graphics& g)
{
	auto e = world.newEntity();
	hexType t {{"spinSprite.png",134,67, 7, 0},100, 0};
	hexTile h {0, 0, 1, t};
	world.addComponent<hexTile>(e, h);
	world.addComponent<pos>(e, h.tSpace());
	world.addComponent<hexPlayerTag>(e,{});

}

std::unordered_map<char, hexType> hexTypes;

void spawnMap (WorldSystems& w,GameState& s, QTree&, Graphics& graphics)
{
	std::ifstream file("map.txt");

	int xOffset = 0;
	int yOffset = 0;

	std::string line;
	while ( std::getline(file, line))
	{
		for (auto c : line)
		{
			//std::print("{0}\n", c);
			if (!hexTypes.contains(c)) continue;
			auto e = w.newEntity();
			hexTile h ={xOffset, yOffset, 0, hexTypes[c]};
			w.addComponent<hexTile>(e, h);
			//w.addComponent<sprite>(e,h.type.s);
			//w.addComponent<renderTag>(e,{});
			w.addComponent<pos>(e, h.tSpace());
			xOffset += 1;
		}
		xOffset = 0;
		yOffset += 1;
	}
}


int main (void)
{
	Game game(1000,1000,1);

	hexTypes['X'] = hexType{ {"hexVoid.png",383,255,0}};
	hexTypes['O'] = hexType{ {"hexUnderVoid00.png",383,255,0}};
	hexTypes['W'] = hexType{ {"HexWood.png",398,255,0}, 0, -13};



	auto render = game.addSystem(renderSystem);
	auto sweep = game.addSystem(sweeper);
	game.addSystem(graphicsRun)->after(render);
	game.addSystem(cameraMove)->after(sweep);
	game.addSystem(hexRender)->before(render);
	game.addSystem(playerHexMove);

	game.addSetup(spawnMap);
	game.addSetup(graphicsSetup);
	game.addSetup(animeSpin);

	game.setup();
	while (true)
	{
		int code = game.gameLoop();
		if (code != 0) break;
	}



	std::print("test\n");

}