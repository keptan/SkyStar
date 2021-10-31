#pragma once 
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
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <vector>
#include "star.h"

//a simple entity component system!
//if you google it you can find out about this
//in an ECS than each object in the game is just a number, that you can associate with different data
//like lots of items will have a 'position' data, and only some will have a hitbox
//the 'Components' of data can be added and removed from the entity at will

//the other part is the 'systems'
//systems are functions that transform the data of valid entities
//for example the render system can go through all the entities that have 'Position' and 'Sprite' components 
//and then draw them on the screen
//
//this kind of system has lots of advantages like Data oriented design ones
//and then making the game is easier because you can compose novel entities easier without making new classes in code
//all the time
//its more flexible
//also its cool and its the in way to make videogames

//the type for Entity and component is just a number, entity 1, entity 2.... etc

//ok we've decided that actually entities need to carry state on child relations so we'll do that instead
using Entity = std::uint32_t;
using ComponentType = std::uint8_t;

//we can only have 32 components in the game! we can easily upgrade this to 64 
const ComponentType MAX_COMPONENTS = 32;
const Entity MAX_ENTITIES = 6400;

//each entity will have a bitset saying which components they have
//this is how systems will find out quickly which entities they are compatible with
using Signature = std::bitset<MAX_COMPONENTS>;

//this is a packed array that we use as the global store of which entities are associated with which components
//well actually all en entity is is just a collection of components right?
//yes
template <typename T, typename E, E Max>
class Packed
{
	std::array<T, Max> res;
	std::queue<E> recycled;
	E count;//why is count a template instead of a size_t?  who knows!

public:

	Packed (void)
		: count(0)
	{}

	//we always try and fill a previously occupied spot in the array 
	//such that the array always remains 'packed', instead of sparse
	//all this does is return an empty spot on the array for us to use
	//this system reads like it could be adapted to a variable amount of entities instead of static
	//but this would incurr some overhead for allocates
	//better than just dumpstering each time we go over the limit
	//as well as overallocating as we do now?
	E create (void)
	{
		if(recycled.size())
		{
			const E use = recycled.front();
			recycled.pop();
			return use;
		}
		//this means we created too many entities and our program explodes
		//fixme(!) arch problem
		assert(count < Max && "Packed array is full!");
		E use = count++;
		return use;
	}

	//log which spaces are free so that we can use them again, yay!
	void destroy (const E id)
	{
		assert(id < Max && "destroying an out of bounds item");
		res[id].reset();
		recycled.push(id);
	}

	//either set, or retieve a calue from our array 
	T& touch (const E id, const std::optional<T> data = std::nullopt)
	{
		if(data == std::nullopt) return res[id];
		res[id] = data.value();
		return res[id];
	}

	//returns how many spaces are currently in use in our array
	size_t size (void)
	{
		return count;
	}

	//here we create an entire forward iterator for using this container
	class Iterator 
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type	= T;
		using pointer           = T*;
		using reference         = T&;

		Packed& res;
		size_t pos;

		public:
		Iterator (Packed& r, const size_t p = 0) : res(r), pos(p)
		{}
		reference operator*() {return res.res[pos];}
		Iterator operator++(int)
		{
			Iterator tmp = *this;
			auto t = pos;
			pos++;
			return tmp;
		}

		Iterator& operator++ (void)
		{
			auto t = pos;
			pos++ ;
			return *this;
		}

		friend bool operator== (const Iterator& a, const Iterator& b)
		{
			return (a.pos == b.pos);
		}

		friend bool operator!= (const Iterator& a, const Iterator& b)
		{
			return (a.pos != b.pos);

		}
	};

	Iterator begin() {return Iterator(*this, 0);}
	Iterator end()	 {return Iterator(*this, count);}
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
//a sparse away is more efficient for iterating large components that most entities DO NOT use!

template<typename T, size_t Max_T = MAX_ENTITIES>
class SparseArray 
{
	//assuming that a cacheline is 64 bytes we can process these one cacheline at a time??
	//im just throwing stuff at the wall here, idk the effecacy of this at all
	//actually we rely on u_long being 64 bit down below, so no!
	static constexpr size_t ChunkSize = sizeof(unsigned long) * 8;
	static constexpr size_t Max = Max_T + (ChunkSize - (Max_T % ChunkSize));
	static constexpr size_t Chunks = Max / ChunkSize;
	//we have an array of bitsets, each bit flag tells us if that position in the array is filled
	//we have an array of optional arrays, each array either doesn't exist or contains some amount
	//of components
	//can we utilize inline storage here for things like sprites? who tf knows!
	std::array<std::bitset<ChunkSize>, Chunks> flags;
	std::array<std::optional<std::array<T, ChunkSize>>, Chunks> res;

	size_t next_valid (const size_t start = 0)
	{
		const size_t startChunk = start / ChunkSize;
		size_t c = startChunk * ChunkSize;

		//taking a position, here we skip through our metaarray
		//until we approach a position where we can actually read data
		for(int i = startChunk; i < Chunks; i++)
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



public:

	SparseArray (void)
	{
		res.fill(std::nullopt);
		flags.fill(0);
	}

	T& insert (const size_t pos, const T in)
	{
		//find what chunk we're inserting into
		//allocate that chunk if necessary (ruh roh!)
	
		const size_t chunk = pos / ChunkSize;
		if(flags[chunk].none()) res[chunk] = std::array<T, ChunkSize>();

		//make sure we set the bitflag to remind us this spot is filled 
		flags[chunk].set(pos - ChunkSize * chunk);
		res[chunk].value()[pos - ChunkSize * chunk] = in;
		return res[chunk].value()[pos - ChunkSize * chunk];
	}

	void remove (const size_t pos)
	{
		const size_t chunk = pos / ChunkSize;
		if(flags[chunk].none()) return; 

		flags[chunk].reset(pos - ChunkSize * chunk);
	}

	//no bounds checking!
	T& get (const size_t pos)
	{
		const size_t chunk = pos / ChunkSize;
		return res[chunk].value()[pos - ChunkSize * chunk];
	}


	class Iterator 
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type		= T;
		using pointer           = T*;
		using reference         = T&;

		SparseArray& res;
		size_t pos;

		public:

		Iterator (SparseArray& r, const size_t p = 0) : res(r)
		{
			 pos = res.next_valid(p);
		};
		reference operator*() {return res.get(pos);}
		Iterator operator++(int)
		{
			Iterator tmp = *this;
			//can we optimize away the constant checks from next_valid() and only
			//call it sometimes?
			pos = res.next_valid(pos);
			return tmp;
		}

		Iterator& operator++ (void)
		{
			pos = res.next_valid(pos);

			return *this;
		}

		friend bool operator== (const Iterator& a, const Iterator& b)
		{
			return (a.pos == b.pos);
		}

		friend bool operator!= (const Iterator& a, const Iterator& b)
		{
			return (a.pos != b.pos);

		}
	};

	Iterator begin() {return Iterator(*this, 0);}
	Iterator end()	 {return Iterator(*this, Max);}
};

//the seq array is a more simpler case where its a component like position where its small
//and lots of entities (nearly all) will contain it
//so we can use this faster datastructure
template <typename T, size_t Max_T = MAX_ENTITIES>
class SeqArray
{
	std::bitset<Max_T> enabled;
	std::array<T, Max_T> res;

	size_t next_valid (const size_t start = 0)
	{
		if(start < 0 || start >= Max_T) return Max_T;
		for(int i = start + 1; i < Max_T; i++)
		{
			if(enabled[i]) return res[i];
		}
	}

public:

	T& insert (const size_t pos, const T in)
	{
		enabled.set(pos);
		res[pos] = in;
		return res[pos];
	}

	void remove (const size_t pos)
	{
		enabled.reset(pos);
	}

	T& get (const size_t pos)
	{
		return res[pos];
	}


	class Iterator 
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type		= T;
		using pointer           = T*;
		using reference         = T&;

		SeqArray& res;
		size_t pos;

		public:

		Iterator (SeqArray& r, const size_t p = 0) : res(r)
		{
			 pos = res.next_valid(p);
		};
		reference operator*() {return res.get(pos);}
		Iterator operator++(int)
		{
			Iterator tmp = *this;
			auto t = pos;
			pos = res.next_valid(pos);
			return tmp;
		}

		Iterator& operator++ (void)
		{
			auto t = pos;
			pos = res.next_valid(pos);

			return *this;
		}

		friend bool operator== (const Iterator& a, const Iterator& b)
		{
			return (a.pos == b.pos);
		}

		friend bool operator!= (const Iterator& a, const Iterator& b)
		{
			return (a.pos != b.pos);

		}
	};

	Iterator begin() {return Iterator(*this, 0);}
	Iterator end()	 {return Iterator(*this, Max_T);}
};


class IComponentArray
{
public:
	virtual ~IComponentArray() = default;
	virtual void entityDestroyed(Entity entity) = 0;
};

//some fancy tag dispatch so that we can use different storage strategies optionally 
template<typename T, typename Default >
class StorageDispatch
{
	template <typename C> static Default test( ... );
	template <typename C> static typename C::StorageStrategy test (typename C::StorageStrategy * );
public:
	using Type = decltype( test<T>(nullptr) );
};

//component array uses this interface V so that we can keep track of entities being wacked
//in which case we should remove the component
//maybe we can just skip this step though if we rely on the packed_list
//uhhh???
template<typename T>
class ComponentArray : public IComponentArray 
{
	using Storage = StorageDispatch<T, SeqArray<T, MAX_ENTITIES>>::Type;
	Storage res;

	public:
	void insert (const Entity e, const T component)
	{
		res.insert(e, component);
	}

	void remove (const Entity e)
	{
		res.remove(e);
	}

	T& get (const Entity e)
	{
		return res.get(e);
	}

	void entityDestroyed(const Entity e) override 
	{
		res.remove(e);
	}
};

//components man does lots of cool magic to help us register and collect
//components easily
class ComponentMan
{
	//string pointer to components
	std::unordered_map<const char*, ComponentType> components;
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays;

	ComponentType ccounter;


public: 

	ComponentMan (void)
		: ccounter(0)
	{}

	template <typename T>
	std::shared_ptr<ComponentArray<T>> getComponentArray (void)
	{
		const char* typeName = typeid(T).name();
		assert(components.find(typeName) != components.end() && "Component not registered before use.");

		return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
	}

	template <typename T>
	ComponentType registerComponent (void)
	{
		const char* typeName = typeid(T).name();
		components.insert({typeName, ccounter});

		componentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});

		return ccounter++;
	}

	template <typename T>
	ComponentType getId (void)
	{
		const char* typeName = typeid(T).name();

		return components[typeName];
	}

	template<typename T>
	void add (const Entity entity, const T component)
	{
		getComponentArray<T>()->insert(entity, component);
	}

	template<typename T>
	T& getComponent (const Entity e)
	{
		return getComponentArray<T>()->get(e);
	}

	void destroy (const Entity entity)
	{
		for (auto const& pair : componentArrays)
		{
			auto const& component = pair.second;
			component->entityDestroyed(entity);
		}
	}
};

//world systems manages entities together
//so if we remove an entity it lets the components, and the list of entities know
class WorldSystems
{
	ComponentMan components;

	public:
	EMan entities;


	Entity newEntity (void)
	{
		auto out = entities.create();
		entities.touch(out, 0);
		return out;
	}

	void killEntity (Entity e)
	{
		entities.destroy(e);
		components.destroy(e);
	}

	template <typename T>
	void registerComponent (void)
	{
		components.registerComponent<T>();
	}

	template <typename T>
	void addComponent (Entity e, T c)
	{
		components.add(e, c);
		auto& signature = entities.touch(e);
		signature.set(components.getId<T>(), true);
	}

	template <typename T>
	void removeComponent (Entity e)
	{
		components.destroy(e);
		auto& signature = entities.touch(e);
		signature.set(components.getId<T>(), false);
	}

	std::vector<Entity> signatureScan (const Signature s)
	{
		std::vector<Entity> acc;
		for(size_t i = 0; i < entities.size(); i++)
		{
			if((entities.touch(i) & s) == s)	acc.push_back(i);
		}

		return acc;
	}

	template<typename... Args>
	Signature createSignature (void)
	{
		Signature out;
		(out.set(getComponentId<Args>()), ... );
		return out;
	}


	template <typename T>
	auto getComponents (void)
	{
		return components.getComponentArray<T>();
	}

	template <typename T>
	ComponentType getComponentId (void)
	{
		return components.getId<T>();
	}

};
		
	


