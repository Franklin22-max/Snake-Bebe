#pragma once

 #include <iostream>
#include <stdint.h>
#include <math.h>
#include <vector>
#include <memory>




#define PI 3.141592653
#define eulers_K 2.7182182

union vec2
{
	struct { double x, y; };
	struct { double w, h; };

	vec2() = default;
	vec2(double x, double y): x(x), y(y){}

	vec2 operator +( const vec2& _1)
	{
		return { x + _1.x, y + _1.y };
	}

	vec2 operator -(const vec2& _1)
	{
		return { x - _1.x, y - _1.y };
	}

	vec2 operator *(double n)
	{
		return { x * n, y * n };
	}

	vec2 operator /(double n)
	{
		return { x / n, y / n };
	}


	bool operator ==(vec2 B)
	{
		return ( x == B.x && y == B.y );
	}
};

struct Circle
{
	double angle = 0;
	vec2 pos;
	double r;
};



struct Polygon
{
	double angle = 0;
	vec2 pos;
	std::vector<vec2> edges;

	Polygon(vec2 position, int sides, double radius)
		:pos(position)
	{
		for (int i = 0; i < sides; i++)
		{
			vec2 edge;
			edge.x = pos.x + cos(2.f * PI * i / sides) * radius;
			edge.y = pos.y + sin(2.f * PI * i / sides) * radius;

			edges.push_back(edge);
		}
	}

	Polygon() = default;

};


struct Rect : public Polygon
{
	Rect(SDL_Rect rect)
	{
		pos.x = rect.x + rect.w / 2;
		pos.y = rect.y + rect.h / 2;

		edges.reserve(4);
		edges.push_back(vec2(rect.x, rect.y));
		edges.push_back(vec2(rect.x + rect.w, rect.y));
		edges.push_back(vec2(rect.x + rect.w, rect.y + rect.h));
		edges.push_back(vec2(rect.x, rect.y + rect.h));
	}
};


inline double get_lenght(vec2 p1, vec2 p2)
{
	auto U = p2 - p1;
	return sqrt(U.x * U.x + U.y * U.y);
}

inline double get_lenght(vec2 U)
{
	return sqrt(U.x * U.x + U.y * U.y);
}

//			Rectangle between points
inline Rect rect_between_points(vec2 p1, vec2 p2, unsigned int thickness)
{
	auto U = p2 - p1;
	double U_mag = sqrt(U.x * U.x + U.y * U.y);

	vec2 U2 = U / U_mag;// reduce u to a unit vector
	vec2 n = { U2.y, -U2.x };// unit perpendicular vector to U

	SDL_Rect r = { 1,1,1,1 };

	Rect rect(r);

	rect.edges[0] = p2 + n * thickness / 2.f;
	rect.edges[1] = rect.edges[0] - U;
	rect.edges[2] = rect.edges[1] - n * thickness;
	rect.edges[3] = rect.edges[2] + U;
	return rect;
}


inline bool is_point_inside_rect(Polygon& p, vec2 point)
{
	double maxX =0, minX = 10000, maxY=0, minY=10000;
	for (auto& edge : p.edges)
	{
		maxX = std::max(maxX, edge.x);
		minX = std::min(maxX, edge.x);
		maxY = std::max(maxY, edge.y);
		minY = std::min(maxY, edge.y);
	}
	//std::cout << point.x <<" , "<< minX << " , " << point.x << " , " << maxX << " , " << point.y << " , " << minY << " , " << point.y << " , " << maxY << "\n";
	return (point.x >= minX && point.x <= maxX && point.y >= minY && point.y <= maxY);
}

inline bool is_point_inside_circle(Circle& c1, vec2 point)
{
	return (((c1.pos.x - point.x) * (c1.pos.x - point.x) + (c1.pos.y - point.y) * (c1.pos.y - point.y)) < (c1.r * c1.r));
}

inline bool is_circle_polygon_colliding(Circle& c, Polygon& p, bool should_separate = false)
{
	bool what_to_return = false;
	for (auto& edge : p.edges)
	{
		
		if (((edge.x - c.pos.x) * (edge.x - c.pos.x) + (edge.y - c.pos.y) * (edge.y - c.pos.y)) < c.r * c.r)
		{
			if (should_separate)
			{
				vec2 d = edge - c.pos;
				c.pos = c.pos - ((d / get_lenght(d)) * c.r) - d;
			}
			what_to_return = true;
		}
			
	}

	return what_to_return;
}

inline bool is_circle_circle_colliding(Circle& c1, Circle& c2, bool should_separate = false)
{
	return (((c1.pos.x - c2.pos.x) * (c1.pos.x - c2.pos.x) + (c1.pos.y - c2.pos.y) * (c1.pos.y - c2.pos.y)) < (c1.r + c2.r) * (c1.r + c2.r));
}