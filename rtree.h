#pragma once
#include "star.h"
#include "geometry.h"
#include <experimental/random>
#include <stack>
#include <algorithm>

/* should be a quad-tree to hold entities that can be queried efficiently using bounding boxes
 * lets take a look if its real or not*/

//the element holds an entity reference and the next element
//in a packed array
//do we need to keep track of next? yes to preserve ordering in the packed array?

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


struct QElement
{
	Rectangle box;
	Entity entity;

	QElement (Rectangle b, Entity e = -1)
		: box(b), entity(e)
	{}
};

//number of elements and the first child in the packed array
struct QNode
{
	Rectangle box;
	std::array< int, 4> children;
	std::vector<QElement> elements;

	QNode (void)
	{
		children.fill(-1);
	};

	QNode (Rectangle b)
		: box(b)
	{
		children.fill(-1);
	}

	QNode (Rectangle b, std::vector<QElement>&& data)
		: box(b), elements( std::move(data))
	{
		children.fill(-1);
	}

	void insert (QElement e)
	{
		elements.push_back(e);
	}


	void rquery (Rectangle& b, Packed<QNode, size_t>& nodes, std::vector< Entity>& collection)
	{
		for(const auto& q: elements) 
		{
			if( b.collides( q.box)) collection.push_back(q.entity);
		}

		for(const auto c: children)
		{
			if(c == -1) continue;
			if( b.collides( nodes[c].box)) 
			{
				nodes[c].rquery(b, nodes, collection);
			}
		}
	}

	void debug_rectangles(Packed<QNode, size_t>& nodes, std::vector<Rectangle>& acc)
	{
			acc.push_back(box);

		for(const auto c : children)
		{
			if(c == -1) continue;
			nodes[c].debug_rectangles(nodes, acc);
		}
	}
};

//our actual api and datastructure
struct QTree 
{
	Packed<QNode, size_t> nodes;
	int root;

	QTree (Rectangle r)
	{
		root = nodes.create();
		nodes[root] = QNode{r};
	}

	void balance (const int n)
	{
		//split elements halfway excluding elements that straddle the divide..
		const double xBoundray = [&]()
		{
			std::sort( nodes[n].elements.begin(), nodes[n].elements.end(), 
					[](const auto& a, const auto& b)
					{
						return (a.box.corner.x + (a.box.w / 2)) < (b.box.corner.x + (b.box.w /2));
					});
			const size_t middle = nodes[n].elements.size() / 2;
			return nodes[n].elements[middle].box.corner.x + nodes[n].elements[middle].box.w / 2;
		}();

		const double yBoundray =[&]()
		{
			std::sort( nodes[n].elements.begin(), nodes[n].elements.end(), 
					[](const auto& a, const auto& b)
					{
						return (a.box.corner.y + (a.box.h / 2)) < (b.box.corner.y + (b.box.h /2));
					});

			const size_t middle = nodes[n].elements.size() / 2;
			return nodes[n].elements[middle].box.corner.y + nodes[n].elements[middle].box.h / 2;
		}();


		std::array< Rectangle, 4> boxes = 
		{
			Rectangle{nodes[n].box.corner,{xBoundray, yBoundray}},
			Rectangle{{xBoundray, nodes[n].box.corner.y}, {nodes[n].box.corner.x + nodes[n].box.w, yBoundray}},
			Rectangle{{nodes[n].box.corner.x, yBoundray}, {xBoundray, nodes[n].box.corner.y + nodes[n].box.h}},
Rectangle{{xBoundray, yBoundray}, {nodes[n].box.corner.x + nodes[n].box.w, nodes[n].box.corner.y + nodes[n].box.h}}
		};

		std::array< std::vector<QElement>,4> boxElements;

		std::vector<QElement> el = nodes[n].elements;
		nodes[n].elements.clear();

		for(const auto& e : el)
		{
			bool flag = true;
			for(int i = 0; i < 4; i++)
			{
				if( boxes[i].contains( e.box))
						{
							boxElements[i].push_back(e);
							flag = false;
							break;
						}
			}
			if(flag) nodes[n].elements.push_back(e);
		}

		for(int i = 0; i < 4; i++)
		{
			if(boxElements[i].size())
			{
				const auto pos = nodes.create();
				nodes[pos] = QNode{boxes[i], std::move( boxElements[i])}; 
				nodes[n].children[i] = pos;
			}
		}

		//nodes[n].elements.shrink_to_fit();
	}

	void rbalance (const int n)
	{
		if(nodes[n].elements.size() < 1 || (nodes[n].box.h < 5) || (nodes[n].box.w < 5)) return;
		balance(n);

		for(int i = 0; i < 4; i++)
		{
			const auto c = nodes[n].children[i];
			if(c != -1) rbalance(c);
		}
	}

	void balance (void)
	{
		rbalance(root);
	}

	void eclear (const int n)
	{
		nodes[n].elements.clear();
		for(int i = 0; i < 4; i++)
		{
			const auto c = nodes[n].children[i];
			if(c != -1) eclear(c);
		}
	}

	void eclear (void)
	{
		eclear(root);
	}

	void qclear ()
	{
		const auto r = nodes[root].box;
		nodes.clear();
		root = nodes.create();
		nodes[root] = QNode{r};
	}

	void rinsert (QElement e, int n)
	{
		for(const auto c : nodes[n].children)
		{
			if(c != -1)
			{
				if( nodes[c].box.contains( e.box))
				{
					rinsert(e, c);
					return;
				}
			}
		}
		nodes[n].elements.push_back(e);
	}

	void insert (QElement e)
	{
		rinsert(e, root);
	}

	std::vector< Entity> query ( Rectangle& b)
	{
		std::vector< Entity> acc;
		nodes[root].rquery(b, nodes, acc);
		std::sort(acc.begin(), acc.end());
		return acc;
	}

	std::vector<Rectangle> rectangles (void)
	{
		std::vector< Rectangle> acc;
		nodes[root].debug_rectangles(nodes, acc);
		return acc;
	}

};

struct hash_pair 
{
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const
    {
        auto hash1 = std::hash<T1>{}(p.first);
auto hash2 = std::hash<T2>{}(p.second);
 
        if (hash1 != hash2) 
	{
            return hash1 ^ hash2;              
        }
         
        // If hash1 == hash2, their XOR is zero.
        return hash1;
    }
};

/*
int main (void)
{
	QTree tree ( Rectangle{{0,}, {1000000,1000000}});

	for(int i = 0; i < 6400; i++)
	{
		int x1 = std::experimental::randint(0, 900000);
		int x2 = std::experimental::randint(x1, x1 + 1000);
		int y1 = std::experimental::randint(0, 900000);
		int y2 = std::experimental::randint(y1, y1 + 1000);

		tree.insert( QElement{ Rectangle{{x1,y1},{x2,y2}}});
	}

	tree.rbalance(tree.root);

	for(int i = 0; i < 12000; i++)
	{

		int x1 = std::experimental::randint(0, 900000);
		int x2 = std::experimental::randint(x1, x1 + 100);
		int y1 = std::experimental::randint(0, 900000);
		int y2 = std::experimental::randint(y1, y1 + 100);

		Rectangle query{{x1,y1},{x2,y2}};
		tree.query(query);
	}

		std::cout << tree.nodes.size() << std::endl;
}
*/
