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

	//no bounds checking!
	T& get (size_t pos)
	{
		const size_t chunk = pos / ChunkSize;

		if(std::get<1>(res[chunk]) == std::nullopt) 
			std::cout << "tried to read a null chunk!: " << pos << '\n';
		return std::get<1>(res[chunk]).value()[(pos - ChunkSize * chunk)];
	}

	size_t next_valid (size_t start = 0)
	{
		size_t startChunk = start / ChunkSize;
		size_t c = startChunk * ChunkSize;
		size_t localOffset = start - c;

		std::cout << startChunk * 64 << '\n';

		for(int i = startChunk; i < res.size(); i++)
		{
			const auto [bitset, data] = res[i];
			if(bitset.none()) 
			{
				c += ChunkSize;

				std::cout << "empty chunk: " << c << "\n";
				continue;
			}

			const auto first = ffsll(bitset.to_ullong()) -1;
			//std::cout << "first: " << first  + c << '\n';

			if(c + first > start) return c + first;

			//std::cout << "reached loop\n";
			//std::cout << i * ChunkSize + localOffset<< '\n';
			for(int q = localOffset + 1; q < ChunkSize; q++)
			{
				if (bitset[q] ) return c + q;
			}

		}

			return Max;
	}

	void remove (size_t pos)
	{
		const size_t chunk = pos / ChunkSize;
		if(std::get<1>(res[chunk]) == std::nullopt) 
			return;

		std::get<0>(res[chunk]).reset((pos - ChunkSize * chunk));
		if(std::get<0>(res[chunk]).none()) std::get<1>(res[chunk]) = std::nullopt;
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

;
