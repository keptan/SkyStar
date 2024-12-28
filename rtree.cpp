#include "star.h"
#include "geometry.h"
#include <experimental/random>
#include <stack>

/* should be a quad-tree to hold entities that can be queried efficiently using bounding boxes
 * lets take a look if its real or not*/

//the element holds an entity reference and the next element
//in a packed array
//do we need to keep track of next? yes to preserve ordering in the packed array?
struct QElement
{
	int x, y;
	Entity entity;
	int next;

	QElement (int x = -1, int y = -1, Entity e = -1, int n = -1)
		: x(x), y(y), entity(e), next(n)
	{}
};

//number of elements and the first child in the packed array
struct QNode
{
	std::array<int, 4> children;
	int elements;
	int size;
	QNode (void)
		:elements(-1), size(0)
	{
		children[0] = -1;
	}
};

//the query struct will be a bounding box that searches for intersections with the smaller boxes
struct QQuery
{
	const int ax;
	const int ay;
	const int bx;
	const int by;

	//descending through a query means shrinking the bounding box through a quadtree
	QQuery descend (const int x, const int y) const
	{
		const int cx = ( ax + bx) / 2;
		const int cy = ( ay + by) / 2;

		if( x <= cx && y <= cy) return { ax, ay, cx, cy};
		if( x >  cx && y <= cy) return { cx, ay, bx, cy};
		if( x >  cx && y >  cy) return { cx, cy, bx, by};
		return { ax, cy, cx, by};
	}

	//checking what quadrant we're in? redundant to the above function
	int quadFind (const int x, const int y) const
	{
		const int cx = ( ax + bx) / 2;
		const int cy = ( ay + by) / 2;

		if( x <= cx && y <= cy) return 0;
		if( x >  cx && y <= cy) return 1;
		if( x >  cx && y >  cy) return 2;
		return 3; 
	}
};

//our actual api and datastructure
struct QTree 
{
	const Rectangle box;

	//nodes are different to elements because there's a fixed amount in a fixed order
	//elements will be constantly spawning in and out of existence
	Packed<QNode, int> nodes;
	Packed<QElement, int> elements;
	int root;

	//we start off with one big node that holds everything
	QTree (Rectangle b)
	: box(b)
	{

		root = nodes.create();
	}

	bool elementFind (const int i, const int x, const int y) const
	{
		if(i == -1) return false;
		if(elements.touch(i).x ==x && elements.touch(i).y == y) return true;
		return elementFind(elements.touch(i).next, x, y);
	}

	void elementDestroy (const int i, const int x, const int y, const int prev = -1)
	{
		if(i == -1) return;
		if(elements.touch(i).x == x && elements.touch(i).y == y)
		{
			if(prev != -1) elements.touch(prev).next = elements.touch(i).next;
			elements.destroy(i);
			return;
		}
		return elementDestroy(elements.touch(i).next, x, y, i);
	}

	void insert (const int x, const int y)
	{
		insertH(x, y, root, {box.corner.x, box.corner.y, box.corner.x + box.w, box.corner.y + box.h});
	}

	bool validateTree (int r)
	{
		for(const auto &n : nodes)
		{
			if(n.children[0] == -1) continue;
			int acc = std::accumulate(
					n.children.begin(), n.children.end(), 0,
					[this](int a, const auto& b) { return a + nodes[b].size;});

			if(acc != n.size) return false;
		}

		return true;
	}

	void insertH (const int x, const int y, const int r, const QQuery query)
	{
		//when we insert we want to avoid splitting the quad depending on the population of our quad
		//and the size. we don't go below size 10 here
		if( (nodes[r].children[0] == -1 && (nodes[r].size < 10) || (query.bx - query.ax) < 10))
		{

			nodes[r].size++;
			int e = elements.create();
			elements.touch(e) = QElement(x, y, -1, nodes[r].elements);
			nodes[r].elements = e;
			return;
		}

		if(nodes[r].children[0] == -1)
		{
			for(int i = 0; i < 4; i++)
			{
				nodes[r].children[i] = nodes.create();
			}

			int elms = nodes[r].elements;
			nodes[r].elements = -1;

			while(elms != -1)
			{
				const auto ex = elements.touch(elms).x;
				const auto ey = elements.touch(elms).y;
				const int	 er = nodes[r].children[query.quadFind(ex, ey)];
				const int old = elements.touch(elms).next;


				elements.touch(elms).next = nodes[er].elements;
				nodes[er].elements = elms;
				nodes[er].size++;
				elms = old;
			}
		}

		nodes[r].size++;
		return insertH(x, y, nodes[r].children[query.quadFind(x, y)], query.descend(x, y));
	}

	//finding a specific node??
	bool find (const int x, const int y) const
	{
		return findH(x, y, 0, {box.corner.x, box.corner.y, box.corner.x + box.h, box.corner.y + box.w});
	}

	bool findH (const int x, const int y, const int r, const QQuery query) const
	{
		if(nodes[r].children[0] == -1) return elementFind(nodes[r].elements, x, y);

		return findH(x, y, nodes[r].children[query.quadFind(x, y)], query.descend(x, y));
	}

	void remove (const int x, const int y)
	{
		return removeH(x, y, 0, {box.corner.x, box.corner.y, box.corner.x + box.h, box.corner.y + box.w});
	}

	//can we balance the quadtree on the way out?
	void removeH (const int x, const int y, const int r,const QQuery query)
	{
		if(nodes[r].children[0] == -1) 
		{
			return elementDestroy(nodes[r].elements, x, y);

			//return elementDestroy(nodes[r].elements, x, y);
		}
		return removeH(x, y, nodes[r].children[query.quadFind(x, y)], query.descend(x, y));
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

int main (void)
{
	QTree qt( Rectangle(Point(0, 0), 1000, 1000));


	for(int i = 0; i < 10000; i++) 
	{
		const int xPos = std::experimental::randint(0, 1000);
		const int yPos = std::experimental::randint(0, 1000);
		qt.insert(xPos,yPos);
	}


	for(int i = 0; i < 10; i = i + 20)
	for(int c = 0; c < 10; c = c + 20)
	{
		//qt.remove(i, c);
	}


	for(int i = 0; i < 10; i = i + 1)
	for(int c = 0; c < 10; c = c + 1)
	{
		if(!qt.find(i, c)) std::cout << "couldn't find: " << i << ' ' << c << std::endl;
	}

	std::cout << qt.validateTree(qt.root) << " validated?" << std::endl;
}
