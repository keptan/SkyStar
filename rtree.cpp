#include "star.h"
#include "geometry.h"
#include <unordered_set>

struct QElement
{
	int x, y;
	Entity entity;
	int next;

	QElement (int x = -1, int y = -1, Entity e = -1, int n = -1)
		: x(x), y(y), entity(e), next(n)
	{}
};

struct QNode
{
	int children;
	int elements;
	QNode (void)
		: children(-1), elements(-1)
	{}
};

struct QQuery
{
	const int ax;
	const int ay;
	const int bx;
	const int by;

	QQuery descend (const int x, const int y) const
	{
		const int cx = (ax + bx) / 2;
		const int cy = (ax + by) / 2;

		if(x <= cx && y <= cy) return { ax, ay, cx, cy};
		if(x >  cx && y <= cy) return { cx, ay, bx, cy};
		if(x >  cx && y >  cy) return { cx, cy, bx, by};
		return { ax, cy, cx, by};

	}

	int quadFind (const int x, const int y) const
	{
		const int cx = (ax + bx) / 2;
		const int cy = (ax + by) / 2;

		if(x <= cx && y <= cy) return 0;
		if(x >  cx && y <= cy) return 1;
		if(x >  cx && y >  cy) return 2;
		return 3; 
	}
};

struct QTree 
{
	Rectangle box;

	std::vector<QNode> nodes;
	Packed<QElement, int> elements;

	QTree (Rectangle b)
	: box(b)
	{
		nodes.push_back( QNode());
	}

	int elementCount (int i, int c = 0)
	{
		if(i == -1) return c;
		return elementCount(elements.touch(i).next, c + 1);
	}

	bool elementFind (int i, int x, int y)
	{
		if(i == -1) return false;
		if(elements.touch(i).x ==x && elements.touch(i).y == y) return true;
		return elementFind(elements.touch(i).next, x, y);
	}

	void elementDestroy (int i, int x, int y, int prev = -1)
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
		return insertH(x, y, 0, {box.corner.x, box.corner.y, box.corner.x + box.w, box.corner.y + box.h});
	}

	void insertH (const int x, const int y, const int r, const QQuery query)
	{
		if(nodes[r].children == -1 && (elementCount(nodes[r].elements) < 2 || (query.bx - query.ax) < 10))
		{

			int e = elements.create();
			elements.touch(e) = QElement(x, y, -1, nodes[r].elements);
			nodes[r].elements = e;
			return;
		}

		if(nodes[r].children == -1)
		{
			nodes[r].children = nodes.size();
			nodes.push_back( QNode());
			nodes.push_back( QNode());
			nodes.push_back( QNode());
			nodes.push_back( QNode());

			int elms = nodes[r].elements;
			nodes[r].elements = -1;

			while(elms != -1)
			{
				const auto ex = elements.touch(elms).x;
				const auto ey = elements.touch(elms).y;
				int  er = -1;

				er = nodes[r].children + query.quadFind(ex, ey);

				const int old = elements.touch(elms).next;
				elements.touch(elms).next = nodes[er].elements;
				nodes[er].elements = elms;
				elms = old;
			}
		}

		return insertH(x, y, nodes[r].children + query.quadFind(x, y), query.descend(x, y));
	}

	bool find (int x, int y)
	{
		return findH(x, y, 0, {box.corner.x, box.corner.y, box.corner.x + box.h, box.corner.y + box.w});
	}

	bool findH (int x, int y, int r, QQuery query)
	{
		if(nodes[r].children == -1) return elementFind(nodes[r].elements, x, y);

		return findH(x, y, nodes[r].children + query.quadFind(x, y), query.descend(x, y));
	}

	void remove (int x, int y)
	{
		return removeH(x, y, 0, {box.corner.x, box.corner.y, box.corner.x + box.h, box.corner.y + box.w});
	}

	void removeH (int x, int y, int r,QQuery query)
	{
		if(nodes[r].children == -1) return elementDestroy(nodes[r].elements, x, y);
		return removeH(x, y, nodes[r].children + query.quadFind(x, y), query.descend(x, y));
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

	for(int i = 0; i < 1000; i = i + 1)
	for(int c = 0; c < 1000; c = c + 1)
	{
		qt.insert(i, c);
	}

	for(int i = 0; i < 1000; i = i + 2)
	for(int c = 0; c < 1000; c = c + 2)
	{
		qt.remove(i, c);
	}



	for(int i = 0; i < 1000; i = i + 1)
	for(int c = 0; c < 1000; c = c + 1)
	{
		if(!qt.find(i, c)) std::cout << "couldn't find: " << i << ' ' << c << std::endl;
	}
}
