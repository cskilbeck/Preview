//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Point : POINT
{
	Point()
	{
	}

	Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	Point(Point const &o)
	{
		x = o.x;
		y = o.y;
	}

	explicit Point(Vec2 const &o)
	{
		x = (int)o.x;
		y = (int)o.y;
	}

	void Clear()
	{
		x = y = 0;
	}

	Point const &operator = (Point const &b)
	{
		x = b.x;
		y = b.y;
		return *this;
	}

	Point const &operator += (Point const &b)
	{
		x += b.x;
		y += b.y;
		return *this;
	}

	Point const &operator -= (Point const &b)
	{
		x -= b.x;
		y -= b.y;
		return *this;
	}

	tstring ToString() const
	{
		return Format(TEXT("(x=%d,y=%d)"), x, y);
	}
};

//////////////////////////////////////////////////////////////////////

inline bool operator == (Point const &a, Point const &b)
{
	return a.x == b.x && a.y == b.y;
}

//////////////////////////////////////////////////////////////////////

inline Point operator + (Point const &a, Point const &b)
{
	return Point (a.x + b.x, a.y + b.y);
}

//////////////////////////////////////////////////////////////////////

inline Point operator - (Point const &a, Point const &b)
{
	return Point (a.x - b.x, a.y - b.y);
}

//////////////////////////////////////////////////////////////////////

inline Point operator - (Point const &a, Vec2 const &b)
{
	return Point ((int)(a.x - b.x), (int)(a.y - b.y));
}

//////////////////////////////////////////////////////////////////////

inline Point operator * (Point const &a, Point const &b)
{
	return Point (a.x * b.x, a.y * b.y);
}

//////////////////////////////////////////////////////////////////////

inline Point operator * (Point const &a, int b)
{
	return Point (a.x * b, a.y * b);
}

//////////////////////////////////////////////////////////////////////

inline Point operator / (Point const &a, Point const &b)
{
	return Point (a.x / b.x, a.y / b.y);
}

//////////////////////////////////////////////////////////////////////

inline Point operator / (Point const &a, int b)
{
	return Point(a.x / b, a.y / b);
}

//////////////////////////////////////////////////////////////////////

template <> inline Point Min(Point a, Point b)
{
	return Point(Min(a.x, b.x), Min(a.y, b.y));
}

//////////////////////////////////////////////////////////////////////

template <> inline Point Max(Point a, Point b)
{
	return Point(Max(a.x, b.x), Max(a.y, b.y));
}

//////////////////////////////////////////////////////////////////////

inline Point Sgn(Point a)
{
	return Point(sgn(a.x), sgn(a.y));
}

//////////////////////////////////////////////////////////////////////
