#pragma once 
#include "star.h"
#include <SDL2pp/Renderer.hh>

using namespace SDL2pp;

//in an ECS than components are just raw data
//we can also put small helper functions in them as long as they're
//"free"

struct pos 
{
	float x = 0;
	float y = 0;
};

struct velocity
{
	float vt;
	float angle;

};

struct sprite 
{
	using StorageStrategy = SparseArray<sprite>;
	std::shared_ptr<Texture> sheet;
	int height = 32;
	int width  = 32;
	int frames = 0;
	int frame  = 0;
};

//tag components too
struct renderTag
{};

struct animationTag
{};
