#pragma once 
#include <bitset>
#include <bit>
#include <cstddef>
#include <iterator>
#include <queue>
#include <array>
#include <optional>
#include <assert.h>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <vector>

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
//this is a packed array that we use as the global store of which entities are associated with which components
//well actually all en entity is is just a collection of components right?
//yes
template <typename T, typename E>
class Packed
{
	std::vector<T> res;
	std::queue<E> recycled;
	E count;//why is count a template instead of a size_t?  who knows!

public:

	Packed (void)
		: count(0)
	{}

	//nuke everything
	void clear (void)
	{
		recycled = std::queue<E>();
		res.clear();
		count = 0;
	}

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
		//assert(count < Max && "Packed array is full!");
		E use = count++;
		res.push_back({});
		return use;
	}

	//log which spaces are free so that we can use them again, yay!
	void destroy (const E id)
	{
		assert(id < res.size() && "destroying an out of bounds item");
		res[id] = {};
		//res[id].reset();
		recycled.push(id);
	}

	//either set, or retrieve a value from our array
	T& touch (const E id)
	{
		return res[id];
	}

	const T& touch (const E id) const
	{
		return res[id];
	}

	const T& operator [] (const E id) const
	{
		return touch(id);
	}

	T& operator [] (const E id)
	{
		return touch(id);
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
		using value_type		= T;
		using pointer           = T*;
		using reference         = T&;

		Packed& res;
		size_t pos;

		public:
		explicit Iterator (Packed& r, const size_t p = 0) : res(r), pos(p)
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

using Entity = uint64_t;
//trying to use a skiplist to skip holes when processing
//if we can detect holes on insertion that is?
//all we need to do is make sure we dont mis-hole, we can unhole and be fine!
//
//a sparse away is more efficient for iterating large components that most entities DO NOT use!

inline uint32_t componentCounter = 0;
inline std::unordered_map<uint32_t, size_t> componentSizes;

template <typename T>
static uint32_t componentId (void)
{
	static const uint32_t id = componentCounter++;
	static const auto inserted = componentSizes.insert(std::make_pair(id, sizeof(T)));
	return id;
}

struct Signature
{
	__uint128_t archetypeID = 0;

	template <typename... Components>
	Signature (void)
	{
		(setComponent<Components>(), ...);
	}

	template <typename T>
	inline void setComponent (void)
	{
		auto bit = componentId<T>();

		archetypeID |= (static_cast<__uint8_t>(1) << bit);
	}

	template<typename... Components>
	void setComponents (void)
	{
		(setComponent<Components>(), ...);
	}

	uint32_t countSet (void) const
	{
		const uint64_t low = static_cast<uint64_t>(archetypeID);
		const uint64_t high = static_cast<uint64_t>(archetypeID >> 64);

		return __builtin_popcountll(low) + __builtin_popcountll(high);
	}

};

template <typename... Components>
Signature createSignature (void)
{
	Signature out;
	(out.setComponent<Components>(), ...);
	return out;
}

struct ComponentRecord
{

	struct alignas(32) EntityRecord
	{
		uint32_t generation;
		uint32_t archetypeID;
		uint32_t rowIndex;
		uint32_t statusFlags;
		Signature signature;
	};

	// 12 bits for the page size (4,096 entries per page)
	static constexpr uint32_t pageShift = 12;
	static constexpr uint32_t pageSize  = 1 << pageShift; // 4096
	static constexpr uint32_t pageMask  = pageSize - 1;

	//full index range
	static constexpr uint32_t directorySize = 1 << (32 - pageShift);

	EntityRecord** pageDirectory;

	uint32_t nextFree = 0xFFFFFFFF; //head of free list
	uint32_t highestPage = 0;

	ComponentRecord (void)
	{
		pageDirectory = static_cast<EntityRecord**>(std::calloc(directorySize, sizeof(EntityRecord*)));

		if (!pageDirectory) {
			throw std::bad_alloc();
		}
	}

	~ComponentRecord(void)
	{
		if (!pageDirectory) return;
		for (uint32_t i = 0; i < directorySize; i++)
		{
			if (pageDirectory[i] != nullptr) free(pageDirectory[i]);
		}
		free(pageDirectory);
	}

	void allocatePage (void)
	{
		//book keep what page we are creating, and that we aren't making a zillion entities...
		const auto pageId = highestPage++;
		assert( pageId < directorySize && "Exceeded max entities");

		//allocate 4099 records, each 32 bytes, alligned to 32 bytes
		void* ptr = nullptr;
		const size_t ps = pageSize * sizeof(EntityRecord);
		if (posix_memalign(&ptr, 32, ps)) assert (false && "Memory allocation failed!");

		const auto page = static_cast<EntityRecord*>(ptr);

		const auto baseIndex = pageId << pageShift;

		//setup free list
		for (uint32_t i = 0; i < pageSize; ++i)
		{
			page[i].generation = 1;
			page[i].signature.archetypeID = 0;

			page[i].rowIndex = (i == pageSize -1) ? 0xFFFFFFFF : (baseIndex + i + 1);
		}

		pageDirectory[pageId] = page;
		nextFree = baseIndex;
	}

	Entity createEntity (const Signature s)
	{
		//make a new page if necessary
		if  (nextFree == 0xFFFFFFFF) allocatePage();

		uint32_t index = nextFree;
		EntityRecord& record = pageDirectory[index >> pageShift][index & pageMask];

		nextFree = record.rowIndex;

		record.signature = s;

		return ( static_cast<uint64_t>(record.generation) << 32| index);
	}

	void destroyEntity (const Entity i)
	{
		const auto index = static_cast<uint32_t>(i);
		auto& record = pageDirectory[index >> pageShift][index & pageMask];

		record.generation++;
		record.signature.archetypeID = 0;

		record.rowIndex = nextFree;
		nextFree = index;
	}

	EntityRecord* getRecordPointer (const Entity i) const
	{
		const auto index = static_cast<uint32_t>(i);
		const auto generation = static_cast<uint32_t>(i >> 32);

		const auto page = index >> pageShift;
		const auto offset = index & pageMask;

		if (!pageDirectory[page]) return nullptr;
		if (pageDirectory[page][offset].generation != generation) return nullptr;

		return &pageDirectory[page][offset];
	}

	//uint64_t createEntity (void)

};

struct World
{

	struct Archetype
	{
		struct Columns
		{
			Signature sig;
			void ** res;

			explicit Columns (Signature s)
				: sig(s), res(nullptr)
			{
				res = new void* [s.countSet()];
			}

			~Columns(void)
			{
				if (res)
				{
					for (int i = 0;  i < sig.countSet(); i++) if (res[i]) free(res[i]);
					free(res);
				}
			}
		};

		Signature sig;
		uint32_t bitToColumn[128];
		uint32_t count;
		std::vector<Columns> pages;

		explicit Archetype( const Signature& s)
			:sig(s), count(0)
		{
			for (int i = 0; i < 128; i++) bitToColumn[i] = 0;
		}

		void allocatePage (void)
		{

			pages.emplace_back( sig);

			const auto low  = static_cast<uint64_t>(sig.archetypeID);
			const auto high = static_cast<uint64_t>(sig.archetypeID >> 64);

			uint32_t current = 0;

			iterateBitsHelper(low, 0, current, pages.back().res);
			iterateBitsHelper(high, 64, current, pages.back().res);


		}

		void iterateBitsHelper (uint64_t val, uint64_t offset, uint32_t& column, void**& columns)
		{
			while (val > 0)
			{
				const uint32_t bit = __builtin_ctzll(val);
				uint32_t global = bit + offset;
				bitToColumn[global] = column;

				posix_memalign(&columns[column], 32, 4096 * componentSizes[global]);
				column++;

				val &= ~(1ULL << bit);
			}
		}

		uint32_t createRecord (void)
		{
			if (count / 4096 <= pages.size()) allocatePage();
			return count++;
		}

		template <typename T>
		T* getColumn (const uint32_t index)
		{
			const auto p = index / 4096;
			const auto i = index - (p * 4096);

			return static_cast<T*>(pages[p].res[i]);
		}

	};

	ComponentRecord entities;
	std::unordered_map<__uint128_t, Archetype> archetypes;

	Entity createEntity (const Signature s)
	{
		if (!archetypes.contains(s.archetypeID)) archetypes.emplace(s.archetypeID, s);
		const auto e = entities.createEntity(s);
		const auto i = archetypes.at(s.archetypeID).createRecord();
		entities.getRecordPointer(e)->rowIndex = i;



		return e;
	}

};



/*
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
	//static component type so that it's shared between archetypes
	inline static std::unordered_map<const char*, ComponentType> components;
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays;

	ComponentType ccounter;


public: 

	ComponentMan (void)
		: ccounter(1)
	{}

	template <typename T>
	ComponentType registerComponent (void)
	{
		const char* typeName = typeid(T).name();
		std::cerr << "registering: " << typeName << std::endl;
		components.insert({typeName, ccounter});
		componentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});

		return ccounter++;
	}


	template <typename T>
	std::shared_ptr<ComponentArray<T>> getComponentArray (void)
	{
		const char* typeName = typeid(T).name();
		//assert(components.find(typeName) != components.end() && "Component not registered before use.");
		if(components.find(typeName) == components.end()) registerComponent<T>();

		return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
	}

	template <typename T>
	ComponentType getId (void)
	{
		const char* typeName = typeid(T).name();

		if(components.find(typeName) == components.end())
		{
			return 0;
		}

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

struct Archetype
{
	ComponentType type;
	ComponentMan components;
};

//world systems manages entities together
//so if we remove an entity it lets the components, and the list of entities know
//this is where we need to put our logic for archetypes..
class WorldSystems
{
	ComponentMan components;

	public:
	EMan entities;


	Entity newEntity (void)
	{
		auto out = entities.create();
		entities[out] = 0;
		return out;
	}

	void killEntity (Entity e)
	{
		entities.destroy(e);
		components.destroy(e);
	}

	/* components are automatically registered though..
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
		if(s == 0) 
		{
			std::cerr << "blank signature!! returning empty array or something" << std::endl;
			return acc;
		}
		for(size_t i = 0; i < entities.size(); i++)
		{
			if((entities.touch(i) & s) == s)	acc.push_back(i);
		}

		return acc;
	}

	template<typename... Args>
	Signature createSignature (void)
	{
		Signature out = 0;
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

	int eCount (void)
	{
		return entities.size();
	}
};
*/
