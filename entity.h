#include <bitset>
#include <queue>
#include <array>
#include <optional>
#include <assert.h>
#include <tuple>
#include <variant>
#include <iostream>

using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 500;

using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

//a packed array of fixed size, but supports adding or removing items
//so that uhhh its contiguous or something ????
template <typename T, typename E, E Max>
class Packed
{
	std::array<T, Max> res;
	std::queue<E> recycled;
	E count;

public:

	Packed (void)
		: count(0)
	{}

	E create (void)
	{
		if(recycled.size())
		{
			const E use = recycled.front();
			recycled.pop();
			return use;
		}
		assert(count < Max && "Packed array is full!");
		E use = count++;
		return use;
	}

	void destroy (const E id)
	{
		assert(id < Max && "destroying an out of bounds item");
		res[id].reset();
		recycled.push(id);
	}

	const T& touch (const E id, const std::optional<T> data = std::nullopt)
	{
		if(data == std::nullopt) return res[id];
		res[id] = data.value();
		return res[id];
	}
};

//packed erray of entity signatures
//an entity signatures determines which properties it holds i guess hehe
//we're actually doing a lot better than the reference iplementation that i stole
//the guy seems to know a lot about ECS programming but is also retarded because he's bad at koding 
//even though he's a professor
using EMan = Packed<Signature, Entity, MAX_ENTITIES>; 

//trying to use a skiplist to skip holes when processing
//if we can detect holes on insertion that is?
//all we need to do is make sure we dont mis-hole, we can unhole and be fine!
//

template<typename T, size_t Max, size_t ChunkSize>
class sparseArray 
{
public:
	static constexpr size_t chunks = Max / ChunkSize;
	std::array< std::tuple< std::bitset<ChunkSize>, std::optional< std::array<T, ChunkSize>>>,chunks> 
	res; 

	sparseArray (void)
	{
		res.fill(std::make_tuple(0, std::nullopt));
	}

	T& insert (size_t pos, const T in)
	{
		const size_t chunk = pos / ChunkSize;
		if(std::get<1>(res[chunk]) == std::nullopt) 
			std::get<1>(res[chunk]) = std::array<T, ChunkSize>();

		std::get<1>(res[chunk]).value()[(pos - ChunkSize * chunk)] = in;
		std::get<0>(res[chunk]).set((pos - ChunkSize * chunk));

		return std::get<1>(res[chunk]).value()[(pos - ChunkSize * chunk)];
	}

	void remove (size_t pos)
	{
		const size_t chunk = pos / ChunkSize;
		if(std::get<1>(res[chunk]) == std::nullopt) 
			return;

		std::get<0>(res[chunk]).reset((pos - ChunkSize * chunk));
		if(std::get<0>(res[chunk]).none()) std::get<1>(res[chunk]) = std::nullopt;
	}

	void print (void)
	{
		int i = 0;
		for(const auto [bitset, opt] : res)
		{
			i++;
			if(bitset.none()) 
			{
				std::cout << "chunk " << i << " empty!\n";
				continue;
			}

			for(int c = 0; c < opt.value().size(); c++)
				if(bitset[c]) std::cout << opt.value()[c] << ' ';
			std::cout << '\n';
		}
	}
	
};

class IComponentArray
{
public:
	virtual ~IComponentArray() = default;
	virtual void EntityDestroyed(Entity entity) = 0;
};

;
