#include "star.h"
#include "geometry.h"

struct QElement
{
	int x, y;
	Entity entity;
	int next;

	QElement (int x = -1, int y = -1, Entity e = -1, int n = -1)
		: entity(e), next(n)
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

struct QTree 
{
	Rectangle box;

	std::vector<QNode> nodes;
	Packed<QElement, int, 6400> elements;

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

	void insert (int x, int y)
	{
		return insertH(x, y, 0, box.corner.x + box.w/2, box.corner.y + box.h/2, box.w/2, box.h/2);
	}

	void insertH (int x, int y, int r, int cx, int cy, int w, int h)
	{
		std::cout << x << ' ' << y << ' ' << r << ' ' << cx << ' ' << cy << ' ' << w << ' ' << h << std::endl;
		if(elementCount(nodes[r].elements) < 5)
		{
			int e = elements.create();
			elements.touch(e) = QElement(x, y, -1, nodes[r].elements);
			nodes[r].elements = e;
			return;
		}


		if(nodes[r].children == -1)
		{
			std::cout << "creating nodes..." << std::endl;
			nodes[r].children = nodes.size();
			nodes.push_back( QNode());
			nodes.push_back( QNode());
			nodes.push_back( QNode());
			nodes.push_back( QNode());
		}

		if(x <= cx && y <= cy) return insertH(x, y, nodes[r].children + 0, cx - w/2,  cy - h/2, w/2, h/2);
		if(x > cx && y <= cy)  return insertH(x, y, nodes[r].children + 1, cx + w/2,  cy - h/2, w/2, h/2);
		if(x <= cx && y > cy) return insertH(x, y, nodes[r].children + 2, cx - w/2,  cy + h/2, w/2, h/2);
		if(x > cx && y > cy) return insertH(x, y, nodes[r].children + 3, cx + w/2,  cy + h/2, w/2, h/2);
	}

};

int main (void)
{
	QTree qt( Rectangle(Point(0, 0), 100, 100));
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
	qt.insert(1,1);
}

