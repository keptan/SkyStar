#include <bitset>
#include <queue>
#include <array>
#include <optional>
#include <assert.h>

using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 5;

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
