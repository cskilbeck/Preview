//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Size : SIZE
{
	Size()
	{
	}

	Size(int w, int h)
	{
		cx = w;
		cy = h;
	}

	explicit Size(Vec2 const &o)
	{
		cx = (int)o.x;
		cy = (int)o.y;
	}

	explicit Size(Point const &o)
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