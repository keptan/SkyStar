#pragma once 
#include <math.h>
#include <assert.h>
#define _USE_MATH_DEFINES




struct pos 
{
	double x = 0;
	double y = 0;
	double rot = 0;

	int distance (const pos& p) const
	{
		return std::sqrt( std::pow( x - p.x, 2) + std::pow(y - p.y, 2));
	}

};

class Point 
{
	public:
	double x;
	double y;

	Point (double x = 0, double y = 0)
		: x(x), y(y)
	{}

	Point (pos p)
		: x(p.x), y(p.y)
	{}

	int distance (const Point& p) const
	{
		return std::sqrt( std::pow( x - p.x, 2) + std::pow(y - p.y, 2));
	}

	bool operator== (const Point& a) const
	{
		if(a.x != x) return false;
		if(a.y != y) return false;
		return true;
	}

	bool operator!= (const Point& a) const
	{
		return !(*this == a);
	}

};

class Velocity 
{
	public:
	double dx, dy;

	Velocity (double x = 0, double y = 0)
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
	double radius;

	Circle (Point p, double r)
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
	double w, h;

	Rectangle (void)
		: corner(0,0), w(0), h(0)
	{}

	Rectangle (Point p, double w, double h)
		: corner(p), w(w), h(h)
	{}

	Rectangle (Point c1, Point c2)
	{
		double lx = std::min(c1.x, c2.x);
		double rx = std::max(c1.x, c2.x);
		double ty = std::min(c1.y, c2.y);
		double by = std::max(c1.y, c2.y);

		corner = Point(lx, ty);
		w 		 = rx - lx;
		h			 = by - ty;
	}

	bool collides (const Point p) const
	{
		if(p.x < corner.x) return false;
		if(p.y < corner.y) return false;
		if(p.x > corner.x + w) return false;
		if(p.y > corner.y + h) return false;

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
		if (l1.y >= r2.y || l2.y >= r1.y) 
			return false; 
	  
		return true; 
	}

	bool contains (const Rectangle r) const
	{
		Point l1 = corner;
		Point l2 = r.corner;
		Point r1(corner.x + w, corner.y + h);
		Point r2(r.corner.x + r.w, r.corner.y + r.h);

		//top corner is inside top corner
		if(l1.x > l2.x ||l1.y > l2.y)
			return false;

		//bottom corner is inside bottom corner
		if(r2.x > r1.x || r2.y > r1.y)
			return false;

		return true;
	}

	bool collides (const Circle circle) const 
	{
		double distX = std::abs(circle.center.x - corner.x - w / 2);
		double distY = std::abs(circle.center.y - corner.y - h / 2);

		if (distX > (w /2 + circle.radius)) return false;
		if (distY > (h /2 + circle.radius)) return false;

		if (distX <= (w / 2)) return true;
		if (distY <= (h / 2)) return true;

		double dx = distX - w/2;
		double dy = distY - h/2;

		return (dx*dx+dy*dy <= (circle.radius * circle.radius));
	}
};
	

