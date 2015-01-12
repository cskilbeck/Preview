//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Size2D : SIZE
{
	Size2D()
	{
	}

	Size2D(int w, int h)
	{
		cx = w;
		cy = h;
	}

	explicit Size2D(Vec2 const &o)
	{
		cx = (int)o.x;
		cy = (int)o.y;
	}

	explicit Size2D(Point2D const &o)
	{
		cx = o.x;
		cy = o.y;
	}

	long Width() const
	{
		return cx;
	}

	long Height() const
	{
		return cy;
	}

	void Clear()
	{
		cx = 0;
		cy = 0;
	}
};

//////////////////////////////////////////////////////////////////////

inline Point2D operator + (Point2D a, Size2D b)
{
	return Point2D(a.x + b.cx, a.y + b.cy);
}

//////////////////////////////////////////////////////////////////////

inline Size2D operator + (Size2D a, Size2D b)
{
	return Size2D(a.cx + b.cx, a.cy + b.cy);
}

//////////////////////////////////////////////////////////////////////

inline Size2D operator + (Size2D a, Point2D b)
{
	return Size2D(a.cx + b.x, a.cy + b.y);
}

//////////////////////////////////////////////////////////////////////

inline Point2D operator - (Point2D a, Size2D b)
{
	return Point2D(a.x - b.cx, a.y - b.cy);
}

//////////////////////////////////////////////////////////////////////

inline Size2D operator - (Size2D a, Size2D b)
{
	return Size2D(a.cx - b.cx, a.cy - b.cy);
}

//////////////////////////////////////////////////////////////////////

inline Size2D operator - (Size2D a, Point2D b)
{
	return Size2D(a.cx - b.x, a.cy - b.y);
}

//////////////////////////////////////////////////////////////////////

inline Size2D operator - (Size2D const &s, Vec2 const &v)
{
	return Size2D((long)(s.cx - v.x), (long)(s.cy - v.y));
}

inline bool operator == (Size2D const &a, Size2D const &b)
{
	return a.cx == b.cx && a.cy == b.cy;
}
