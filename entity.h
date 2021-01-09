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

using Entity = std::uint32_t;
using ComponentType = std::uint8_t;

const ComponentType MAX_COMPONENTS = 32;
const Entity MAX_ENTITIES = 6400;

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

	T& touch (const E id, const std::optional<T> data = std::nullopt)
	{
		if(data == std::nullopt) return res[id];
		res[id] = data.value();
		return res[id];
	}

	size_t size (void)
	{
		return count;
	}

	class Iterator 
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type		= T;
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


template<typename T, size_t Max_T = MAX_ENTITIES>
class SparseArray 
{
	static constexpr size_t ChunkSize = 64;
	static constexpr size_t Max = Max_T + (ChunkSize - (Max_T % ChunkSize));
	static constexpr size_t Chunks = Max / ChunkSize;
	std::array<std::bitset<ChunkSize>, Chunks> flags;
	std::array<std::optional<std::array<T, ChunkSize>>, Chunks> res;

	size_t next_valid (const size_t start = 0)
	{
		const size_t startChunk = start / ChunkSize;
		size_t c = startChunk * ChunkSize;

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
		const size_t chunk = pos / ChunkSize;
		if(flags[chunk].none()) res[chunk] = std::array<T, ChunkSize>();

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
	Iterator end()	 {return Iterator(*this, Max);}
};

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




template<typename T, typename Default >
class StorageDispatch
{
	template <typename C> static Default test( ... );
	template <typename C> static typename C::StorageStrategy test (typename C::StorageStrategy * );
public:
	using Type = decltype( test<T>(nullptr) );
};

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

class ComponentMan
{
	//string pointer to components
	std::unordered_map<const char*, ComponentType> components;
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays;

	ComponentType ccounter;


public: 

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

		return ++ccounter;
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
		
	


