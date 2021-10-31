#pragma once 
#include <math.h>





struct pos 
{
	float x = 0;
	float y = 0;
	float rot = 0;

	int distance (const pos& p) const
	{
		return std::sqrt( std::pow( int(x) - int(p.x), 2) + std::pow(int(y) - int(p.y), 2));
	}
};

class Point 
{
	public:
	int x = 0;
	int y = 0;

	Point (int x, int y)
		: x(x), y(y)
	{}

	Point (pos p)
		: x(int(p.x)), y(int(p.y))
	{}

	int distance (const Point& p) const
	{
		return std::sqrt( std::pow( int(x) - int(p.x), 2) + std::pow(int(y) - int(p.y), 2));
	}
};

class Velocity 
{
	public:
	int dx, dy;

	Velocity (int x = 0, int y = 0)
		: dx(x), dy(y)
	{}

	double magnitude (void)
	{
		//a^2 + b^2 = c^2
		return std::sqrt( std::pow(dx, 2) +  std::pow(dy, 2));
	}

	Velocity operator+ (const Velocity& rhs) const
	{
		Velocity out;
		out.dx = dx + rhs.dx;
		out.dy = dy + rhs.dy;
		return out;
	}

	Velocity normalize (double scale = 1)
	{
		Velocity out;
		const auto m = magnitude();
		if(m > 0)
		{
			out.dx = (dx / m) * scale;
			out.dy = (dy / m) * scale;
		}
		return out;
	}
};

class Circle
{
	public:
	Point center;
	int radius;

	Circle (Point p, int r)
		: center(p), radius(r)
	{}
};

/*
class Line
{
	public:
	Point start, end;

	Line (Point s, Point e)
		: start(s), end(e)
	{}
};
*/

class Rectangle 
{
	public:
	Point corner;
	int w, h;

	Rectangle (Point p, int w, int h)
		: corner(p), w(w), h(h)
	{}

	bool collides (const Point p) const
	{
		if(p.x < corner.x || p.y < corner.y) return false;
		if((p.x + w > corner.x + w) || (p.y + h < corner.y + h)) return false;

		return true;
	}

	bool collides (const Rectangle r) const
	{
		Point l1 = corner;
		Point l2 = r.corner;
		Point r1(corner.x + w, corner.y + h);
		Point r2(r.corner.x + r.w, r.corner.y + r.h);

	 // If one rectangle is on left side of other 
		if (l1.x >= r2.x || l2.x >= r1.x) 
			return false; 
	  
		// If one rectangle is above other 
		if (l1.y <= r2.y || l2.y <= r1.y) 
			return false; 
	  
		return true; 
	}

	bool collides (const Circle circle) const 
	{
		int distX = std::abs(circle.center.x - corner.x - w / 2);
		int distY = std::abs(circle.center.y - corner.y - h / 2);

		if (distX > (w /2 + circle.radius)) return false;
		if (distY > (h /2 + circle.radius)) return false;

		if (distX <= (w / 2)) return true;
		if (distY <= (h / 2)) return true;

		int dx = distX - w/2;
		int dy = distY - h/2;

		return (dx*dx+dy*dy <= (circle.radius * circle.radius));
	}

};
	

