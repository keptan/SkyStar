#pragma once 
#include <bitset>
#include <iterator>
#include <queue>
#include <array>
#include <optional>
#include <assert.h>
#include <unordered_map>
#include <vector>
#include <print>

//we completely tore this out and remade it from scratch
//it's an ECS system that can have up to 128 components, in arbitrary combinations
//and 4 billion entitities or whatever
//stores components using archetypes because its faster than not doing that
//before adding archetypes we were using skiplists instead which is dumb


using Entity = uint64_t;
//an entity is a 32bit index with a 32bit generation counter so we can deadcheck them

//this logic allows us to tally up our 128 components as they first used, type based incremental counter
inline uint32_t componentCounter = 0;
inline std::unordered_map<uint32_t, size_t> componentSizes;

template <typename T>
static uint32_t componentId (void)
{

	static const uint32_t id = componentCounter++;
	static const auto inserted = componentSizes.insert(std::make_pair(id, sizeof(T)));

	return id;
}

//signature of what components an entity has, determins what archetype it goes into
//also determines which archetypes will get queried by our systems if the masks overlap
struct Signature
{
	__uint128_t archetypeID = 0;

	template <typename... Components>
	Signature (void)
	{
		(setComponent<Components>(), ...);
	}

	Signature (void)
	{

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

// All entities no matter what archetype are registered here
// this record connects entitiy numbers to the archetype storage rows
// so we can quickly access an archetype from the corresponding entity
struct EntityRegister
{

	struct alignas(32) EntityRecord
	{
		uint32_t generation; // used for validation
		uint32_t rowIndex; // used to directly access the index in the archetype storage
		uint32_t statusFlags; // not implemented yet, but will let us to flag things without moving storage
		Signature signature; // the component mask mentioned above
	};

	// 12 bits for the page size (4,096 entries per page)
	static constexpr uint32_t pageShift = 12;
	static constexpr uint32_t pageSize  = 1 << pageShift; // 4096
	static constexpr uint32_t pageMask  = pageSize - 1;

	//full index range
	static constexpr uint32_t directorySize = 1 << (32 - pageShift);

	//all our paged storage, pages of 4096 entityrecords all the way to 4 bazillion
	EntityRecord** pageDirectory;

	uint32_t nextFree = 0xFFFFFFFF; //head of free list
	uint32_t highestPage = 0;

	EntityRegister (void)
	{
		pageDirectory = static_cast<EntityRecord**>(std::calloc(directorySize, sizeof(EntityRecord*)));

		if (!pageDirectory) {
			throw std::bad_alloc();
		}
	}

	~EntityRegister(void)
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

//the handle for the ECS system
//coordinates archetype storage and entity id handouts
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
				if (s.archetypeID)
				{
					res = new void* [s.countSet()];
					//for (int i = 0; i < s.countSet(); i++) res[i] = nullptr;
				}
			}

			Columns (Columns&& a) noexcept
				: sig(a.sig),res(a.res)
			{
				a.res = nullptr;
				a.sig = Signature();
			}

			Columns& operator =(Columns&& a) noexcept
			{
				if (this != &a)
				{
					this->clear();
					this->res = a.res;
					this->sig = a.sig;

					a.res = nullptr;
					a.sig = Signature();
				}
				return *this;
			}

			void clear (void)
			{
				if (res)
				{
					for (uint32_t i = 0; i < sig.countSet(); i++)
					{
						free(res[i]);
					}
					delete[] res;
					res = nullptr;
				}
			}

			~Columns(void)
			{
				clear();
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

				auto e = posix_memalign(&columns[column], 32, 4096 * componentSizes[global]);
				column++;

				val &= ~(1ULL << bit);
			}
		}

		uint32_t createRecord (void)
		{
			if ((count / 4096) >= pages.size())
			{
				//std::print("{}, {}\n", count/4096, pages.size());
				allocatePage();
			}
			return count++;
		}

		template <typename T>
		void setComponent (const uint32_t index, T&& value)
		{
			const auto page = index / 4096;
			const auto i = index - (page * 4096);
			const auto bit = bitToColumn[componentId<T>()];
			T* col = static_cast<T*>( pages[page].res[bit]);
			new (&col[i]) T(std::forward<T>(value));
		}
	};

	EntityRegister entities;
	std::unordered_map<__uint128_t, Archetype> archetypes;

	Entity createEntity (const Signature s)
	{
		if (!archetypes.contains(s.archetypeID)) archetypes.emplace(s.archetypeID, s);
		const auto e = entities.createEntity(s);
		const auto i = archetypes.at(s.archetypeID).createRecord();
		entities.getRecordPointer(e)->rowIndex = i;

		return e;
	}
	template <typename... Ts>
	Entity emplaceEntity (Ts&&... components)
	{
		Signature s = createSignature<std::decay_t<Ts>...>();
		if (!archetypes.contains(s.archetypeID)) archetypes.emplace(s.archetypeID, s);
		auto& a = archetypes.at(s.archetypeID);
		const auto e = entities.createEntity(s);
		const auto i = a.createRecord();
		entities.getRecordPointer(e)->rowIndex = i;

		(a.setComponent<std::decay_t<Ts>>(i, std::forward<Ts>(components)), ...);

		return e;
	}

	template <typename ... Ts>
	struct Query
	{
		Signature s = createSignature<Ts...>();
		std::vector<Archetype*> matching;

		explicit Query (World& w)
		{
			for (auto &arch : w.archetypes)
			{
				if (( arch.second.sig.archetypeID & s.archetypeID) == s.archetypeID)
				{
					matching.push_back(&arch.second);
				}
			}
		}
	};

	template<typename... Ts, typename F>
	void each (Query<Ts...> query, F&& func)
	{
		for (Archetype* arch : query.matching)
		{
			auto total = arch->count;

			for ( uint32_t p = 0; p < arch->pages.size(); p++)
			{
				auto& page = arch->pages[p];

				auto columns = std::make_tuple(
					static_cast<Ts*>(page.res[arch->bitToColumn[componentId<Ts>()]])...
					);

				uint32_t entitiesInPage = std::min(4096u, total - (p * 4096));

				for (uint32_t i = 0; i < entitiesInPage; i++)
				{
					func(std::get<Ts*>(columns)[i]...);
				}

			}
		}
	}
};
