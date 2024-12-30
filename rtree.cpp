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

	void balance ( Packed<QNode, size_t>& nodes)
	{
		//split elements halfway excluding elements that straddle the divide..
		const int xBoundray = [this]()
		{
			std::sort( elements.begin(), elements.end(), 
					[this](const auto a, const auto b)
					{
						return (a.box.corner.x + a.box.w / 2) > (b.box.corner.x + b.box.w /2);
					});
			const size_t middle = elements.size() / 2;
			return elements[middle].box.corner.x + elements[middle].box.w / 2;
		}();
		const int yBoundray =[this]()
		{
			std::sort( elements.begin(), elements.end(), 
					[this](const auto a, const auto b)
					{
						return (a.box.corner.y + a.box.h / 2) > (b.box.corner.y + b.box.h /2);
					});

			const size_t middle = elements.size() / 2;
			return elements[middle].box.corner.y + elements[middle].box.h / 2;
		}();

		std::array< Rectangle, 4> boxes = 
		{
			Rectangle{box.corner,{xBoundray, yBoundray}},
			Rectangle{{xBoundray, box.corner.y}, {box.corner.x + box.w, yBoundray}},
			Rectangle{{box.corner.x, yBoundray}, {xBoundray, box.corner.y + box.h}},
			Rectangle{{xBoundray, yBoundray}, {box.corner.x + box.w, box.corner.y + box.h}}
		};

		std::array< std::vector<QElement>,4> boxElements;

		std::vector<QElement> el = elements;
		elements.clear();

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
			if(flag) elements.push_back(e);
		}

		for(int i = 0; i < 4; i++)
		{
			if(boxElements[i].size())
			{
				const auto n = nodes.create();
				nodes[n] = QNode{boxes[i], std::move( boxElements[i])}; 
				children[i] = n;
			}
		}
	}

	void rbalance (Packed<QNode, size_t>& nodes)
	{
		if(elements.size() < 10 || (box.h < 10) || (box.w < 10)) return;
		balance(nodes);

		for(const auto c : children) if(c != -1) nodes[c].rbalance(nodes);
	}

};

//our actual api and datastructure
struct QTree 
{
	//nodes are different to elements because there's a fixed amount in a fixed order
	//elements will be constantly spawning in and out of existence
	std::vector<int> nodes;
	int root;

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

int main (void)
{
	Packed<QNode, size_t> nodes;
	int root = nodes.create();
	nodes[root] = {{{0,0},{1000,1000}}};

	for(int i = 0; i < 1000; i++)
	{
		const int a1 = std::experimental::randint(0, 199);
		const int a2 = a1 + std::experimental::randint(1, 10);
		const int b1 = std::experimental::randint(0, 199);
		const int b2 = b1 + std::experimental::randint(1, 10);
		const Rectangle r{{a1, b1}, {a2, b2}};
		nodes[root].insert(r);
	}
	nodes[root].rbalance(nodes);
	std::cout << nodes.size() << " nodes created" << std::endl;
}
