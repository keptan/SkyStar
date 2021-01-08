#include <bitset>
#include <bit>
#include <cstddef>
#include <iterator>
#include <queue>
#include <array>
#include <optional>
#include <assert.h>
#include <tuple>
#include <variant>
#include <iostream>

#include <string.h>
int ffsll(long long int i);


using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 6400;

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
	std::array<std::bitset<ChunkSize>, chunks> flags;
	std::array<std::optional<std::array<T, ChunkSize>>, chunks> res;

	sparseArray (void)
	{
		res.fill(std::nullopt);
		flags.fill(0);
	}

	T& insert (size_t pos, const T in)
	{
		const size_t chunk = pos / ChunkSize;
		if(flags[chunk].none()) res[chunk] = std::array<T, ChunkSize>();
		flags[chunk].set(pos - ChunkSize * chunk);
		res[chunk].value()[pos - ChunkSize * chunk] = in;

		return res[chunk].value()[pos - ChunkSize * chunk];
	}

	void remove (size_t pos)
	{
		const size_t chunk = pos / ChunkSize;
		if(flags[chunk].none()) return; 

		flags[chunk].reset(pos - ChunkSize * chunk);
	}

	//no bounds checking!
	T& get (size_t pos)
	{
		const size_t chunk = pos / ChunkSize;

		return res[chunk].value()[pos - ChunkSize * chunk];
	}

	size_t next_valid (size_t start = 0)
	{
		size_t startChunk = start / ChunkSize;
		size_t c = startChunk * ChunkSize;

		for(int i = startChunk; i < chunks; i++)
		{
			const auto& bitset = flags[i];
			if(bitset.none())
			{
				c += ChunkSize;
				continue;
			}

			const size_t end = std::countl_zero(bitset.to_ullong());
			const size_t begin = std::countr_zero(bitset.to_ullong());
			for(int q = begin; q < ChunkSize - end; q++)
			{
			
				if(bitset[q] && c + q > start) 
				{
					return c + q;
				}
			}
			c+= ChunkSize;
		}
		return c;

	}



	class iterator 
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type		= T;
		using pointer           = T*;
		using reference         = T&;

		sparseArray& res;
		size_t pos;

		public:

		iterator (sparseArray& r, size_t p = 0) : res(r)
		{
			 pos = res.next_valid(p);
		};
		reference operator*() {return res.get(pos);}
		iterator operator++(int)
		{
			iterator tmp = *this;
			auto t = pos;
			pos = res.next_valid(pos);
			return tmp;
		}

		iterator& operator++ (void)
		{
			auto t = pos;
			pos = res.next_valid(pos);

			return *this;
		}

		friend bool operator== (const iterator& a, const iterator& b)
		{
			return (a.pos == b.pos);
		}

		friend bool operator!= (const iterator& a, const iterator& b)
		{
			return (a.pos != b.pos);

		}
	};

	iterator begin() {return iterator(*this, 0);}
	iterator end()	 {return iterator(*this, Max);}


	int print (void)
	{
		int i = 0;
		for(auto [bitset, opt] : res)
		{
			if(bitset.none()) 
			{
				i += ChunkSize;
				continue;
			}

			const size_t consq = ffsll(bitset.to_ullong()) - 1;
			const size_t conqEnd = std::countl_zero(bitset.to_ullong());
			i += consq;
			i += conqEnd;

			for(int c = consq; c < conqEnd; c++)
				if(bitset[c])  opt.value()[c] = c+ 1;
		}
		return i;
	}
	
};

class IComponentArray
{
public:
	virtual ~IComponentArray() = default;
	virtual void EntityDestroyed(Entity entity) = 0;
};

template<typename T>
class SparseComponentArray : public IComponentArray 
{
	sparseArray<T, MAX_ENTITIES, 64> res;
	public:
	void insert (Entity e, T component)
	{
		res.insert(e, component);
	}

	void remove (Entity e)
	{
		res.remove(e);
	}

	T& GetData (Entity entity)
	{
		return res.get(entity);
	}

	void EntityDestroyed(Entity e) override 
	{
		res.remove(e);
	}
};
	


