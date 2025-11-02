#pragma once
#include <any>
#include <memory>

#include "star.h"
#include <SDL2pp/Renderer.hh>
#include <SDL2pp/SDL2pp.hh>
#include <filesystem>
#include <list>
#include <print>

#include "rtree.h"

struct Graphics;

enum class InputMask
{
	None = 0x00,
	Up	 = 1 << 1,
	Down = 1 << 2,
	Left = 1 << 3,
	Right = 1 << 4,
	Attack = 1 << 5,
	Quit = 1 << 6,
	LShift = 1 << 7,
	PKey   = 1 << 8,
	Space  = 1 << 9,
};

inline InputMask operator | (InputMask lhs, InputMask rhs)
{
	using T = std::underlying_type_t<InputMask>;
	return static_cast<InputMask>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline InputMask operator & (InputMask lhs, InputMask rhs)
{
	using T = std::underlying_type_t<InputMask>;
	return static_cast<InputMask>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline InputMask& operator ^= (InputMask& lhs, InputMask rhs)
{	
	using T = std::underlying_type_t<InputMask>;
	lhs = static_cast<InputMask>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
	return lhs;
}

inline InputMask& operator |= (InputMask& lhs, InputMask rhs)
{	
	using T = std::underlying_type_t<InputMask>;
	lhs = static_cast<InputMask>(static_cast<T>(lhs) | static_cast<T>(rhs));
	return lhs;
}

struct GameState
{
	InputMask input = InputMask::None;

	unsigned int frameCount = 0;
	unsigned long int time = 0;
	unsigned int frameTime = 0;
	unsigned int averageFrameTime = 0;
};


void sweeper (WorldSystems& world, GameState& state, QTree& space, Graphics& graphics)
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		 if (event.type == SDL_KEYDOWN) 
		 {
				switch (event.key.keysym.sym) 
				{
					case SDLK_w: state.input |= InputMask::Up; break;
					case SDLK_a: state.input |= InputMask::Left; break;
					case SDLK_s: state.input |= InputMask::Down; break;
					case SDLK_d: state.input |= InputMask::Right; break;
					case SDLK_q: state.input |= InputMask::Quit; break;
					case SDLK_LSHIFT: state.input |= InputMask::LShift; break;
					case SDLK_p: state.input |= InputMask::PKey; break;
					case SDLK_SPACE: state.input |= InputMask::Space; break;
				}

		} 
		else if (event.type == SDL_KEYUP)
		{
				switch (event.key.keysym.sym)
				{
					case SDLK_w: state.input ^= InputMask::Up; break;
					case SDLK_a: state.input ^= InputMask::Left; break;
					case SDLK_s: state.input ^= InputMask::Down; break;
					case SDLK_d: state.input ^= InputMask::Right; break;
					case SDLK_q: state.input ^= InputMask::Quit; break;
					case SDLK_LSHIFT: state.input ^= InputMask::LShift; break;
					case SDLK_p: state.input ^= InputMask::PKey; break;
					case SDLK_SPACE: state.input ^= InputMask::Space; break;
				}
		}
	}
}

struct Graphics
{
	SDL2pp::SDL   		sdl;
	SDL2pp::SDLTTF 	sdl_ttf;
	SDL2pp::Window		window;
	SDL2pp::Renderer	rendr;

	std::unordered_map<std::string, std::shared_ptr<SDL2pp::Texture>> textures;

	Graphics (int width, int height)
		: sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO),
		  window("Demo..", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			 width, height,
			 SDL_WINDOW_RESIZABLE),
		  rendr(window, -1, SDL_RENDERER_ACCELERATED)
	{
		for (const auto& entry : std::filesystem::directory_iterator(DATA_PATH))
		{
			if (entry.is_regular_file())
			{
				std::cerr << "registering texture: " << entry.path().filename() << "\n";
				textures[entry.path().filename()] = std::make_shared< SDL2pp::Texture>(rendr, entry.path());
			}
		}
	}

	std::shared_ptr<SDL2pp::Texture> getTexture (const std::string& name)
	{
		assert( textures.contains(name));
		return textures[name];
	}
};

//stuff that test.cpp has
//register systems
//hold world and stuff so we can spawn entities and stuff
//and we need a scripting and scene structure to load levels in
//buuuhhh
//systems are functions that operate on a subset of entities

struct SystemRegister
{
	using SystemFunction = void(*)(WorldSystems& world, GameState& state, QTree& space, Graphics& graphics);

	struct SystemGraphNode
	{
		SystemFunction system;
		std::unordered_set< std::shared_ptr< SystemGraphNode>> inSet;
		std::unordered_set< std::shared_ptr< SystemGraphNode>> outSet;


		explicit SystemGraphNode (const SystemFunction f)
			: system(f)
		{}

		SystemGraphNode& after (const std::shared_ptr< SystemGraphNode>& f)
		{
			inSet.insert(f);
			return *this;
		}

		SystemGraphNode& before (const std::shared_ptr< SystemGraphNode>& f)
		{
			outSet.insert(f);
			return *this;
		}
	};

	std::unordered_set< std::shared_ptr<SystemGraphNode>> systems;
	std::unordered_set< std::shared_ptr<SystemGraphNode>> setup;
	std::vector<SystemFunction> orderedSystems;
	std::vector<SystemFunction> orderedSetup;

	std::shared_ptr< SystemGraphNode> add (SystemFunction f)
	{
		auto p = std::make_shared<SystemGraphNode>(f);
		systems.insert(p);
		return p;
	}

	std::shared_ptr< SystemGraphNode> addSetup (SystemFunction f)
	{
		auto p = std::make_shared<SystemGraphNode>(f);
		setup.insert(p);
		return p;
	}

	//we need to do a topographic sort on our nodes so they all execute in the order they want
	//this is where we can invalidate peoples before and after specifications too, it would be cool
	//if we could do it on compiler time though
	//using khan's algorithm is simple, we will get an infinite loop here derp
	std::vector<SystemFunction> topoSort( std::unordered_set< std::shared_ptr<SystemGraphNode>>& set)
	{
		std::vector<SystemFunction> res;
		std::unordered_set< std::shared_ptr<SystemGraphNode>> inLess;

		//pre-process 1
		for (auto &node : set)
		{
			for (auto &n : node->outSet)
			{
				n->inSet.insert(node);
			}
		}

		//pre-process 2
		for (auto& node : set) if (node->inSet.empty()) inLess.insert(node);

		//pre-process 3
		while (!inLess.empty())
		{
			auto node = *inLess.begin();
			inLess.erase(node);
			set.erase(node);
			res.push_back(node->system);

			for (auto& n : node->outSet)
			{
				n->inSet.erase(node);
				if (n->inSet.empty()) inLess.insert(n);
			}
		}

		assert( set.empty() && "graph is cyclical");
		return res;
	}

	void sortSystems (void)
	{
		orderedSystems = topoSort(systems);
		orderedSetup = topoSort(setup);
	}

};

class Game
{
	int sceneHeight, sceneWidth, windowScale;


	WorldSystems	world;
	Graphics			graphics;
	GameState		state;
	SystemRegister systems;
	QTree				space;

public:
	Game (void)
		: sceneHeight(480), sceneWidth(640), windowScale(2),
		graphics(sceneWidth * windowScale, sceneHeight * windowScale),
		space( Rectangle( {0, 0}, sceneWidth, sceneHeight))
	{}

	std::shared_ptr< SystemRegister::SystemGraphNode> addSystem ( SystemRegister::SystemFunction f)
	{
		return systems.add(f);
	}

	std::shared_ptr< SystemRegister::SystemGraphNode> addSetup ( SystemRegister::SystemFunction f)
	{
		return systems.addSetup(f);
	}

	int setup (void)
	{
		systems.sortSystems();

		for (const auto s : systems.orderedSetup)
		{
			s(world, state, space, graphics);
		}

		return 0;
	}

	int gameLoop (void)
	{
		graphics.rendr.SetDrawColor(20, 0, 0, 255);
		graphics.rendr.Clear();
		const auto tick = SDL_GetTicks();
		state.frameTime = tick - state.time;
		state.time = tick;

		if ((state.input & InputMask::Quit) == InputMask::Quit) return 1;

		for (const auto s : systems.orderedSystems)
		{
			s(world, state, space, graphics);
		}
		graphics.rendr.Present();

		std::print( "{0}\n", world.entities.size());

		state.frameCount++;
		const auto endFrame = SDL_GetTicks();
		const auto frameCost = endFrame - tick;
		state.averageFrameTime += frameCost;
		SDL_Delay(frameCost >= 7 ? 0 : 7 - frameCost);
		return 0;
	}

	//auto register = game.registerSystem(blah)
	//auto register = game.regiserSystem(blah).before(blah,blah).after(blah, blah);


};
