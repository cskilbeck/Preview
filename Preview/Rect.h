
struct Rect2D
{
	Point2D	mTopLeft;
	Size2D	mSize;

	Rect2D()
	{
	}

	Rect2D(int x, int y, int width, int height)
		: mTopLeft(x, y)
		, mSize(width, height)
	{
	}

	Rect2D(RECT &rc)
		: mTopLeft(rc.left, rc.top)
		, mSize(rc.right - rc.left, rc.bottom - rc.top)
	{
	}

	Rect2D(Point2D topLeft, Size2D size)
		: mTopLeft(topLeft)
		, mSize(size)
	{
	}

	Rect2D(Vec2 topLeft, Vec2 size)
		: mTopLeft(topLeft)
		, mSize(size)
	{
	}

	void SetRECT(RECT &rc)
	{
		SetRect(&rc, mTopLeft.x, mTopLeft.y, mTopLeft.x + mSize.w, mTopLeft.y + mSize.h);
	}

	bool Contains(Point2D p) const
	{
		return p.x >= mTopLeft.x && p.x < mTopLeft.x + mSize.w && p.y >= mTopLeft.y && p.y < mTopLeft.y + mSize.h;
	}
};

struct Rect
{
	Rect();
	Rect(Vec2 origin, Vec2 size);
	Rect(float x, float y, float width, float height);
	void Set(float x, float y, float width, float height);
	void Set(Vec2 origin, Vec2 size);
	float GetWidth() const;
	float GetHeight() const;
	void SetWidth(float width);
	void SetHeight(float height);
	void SetOrigin(Vec2 topLeft);
	Vec2 GetOrigin() const;
	void SetSize(Vec2 const size);
	Vec2 const &GetSize() const;
	float Right() const;
	float Left() const;
	float Bottom() const;
	float Top() const;
	void Move(Vec2 offset);
	void Inflate(Vec2 amount);
	void Deflate(Vec2 amount);
	bool IsEmpty() const;
	Rect Intersect(Rect const &o) const;
	Rect Union(Rect const &o) const;

private:

	Vec2	mOrigin;
	Vec2	mSize;
};

inline Rect::Rect()
{
}

inline Rect::Rect(Vec2 origin, Vec2 size)
	: mOrigin(origin)
	, mSize(size)
{
}

inline Rect::Rect(float x, float y, float width, float height)
	: mOrigin(x, y)
	, mSize(width, height)
{
}

inline void Rect::Set(float x, float y, float width, float height)
{
	mOrigin = Vec2(x, y);
	mSize = Vec2(width, height);
}

inline void Rect::Set(Vec2 origin, Vec2 size)
{
	mOrigin = origin;
	mSize = size;
}

inline float Rect::GetWidth() const
{
	return mSize.x;
}

inline float Rect::GetHeight() const
{
	return mSize.y;
}

inline void Rect::SetWidth(float width)
{
	mSize.x = width;
}

inline void Rect::SetHeight(float height)
{
	mSize.y = height;
}

inline void Rect::SetOrigin(Vec2 const origin)
{
	mOrigin = origin;
}

inline Vec2 Rect::GetOrigin() const
{
	return mOrigin;
}

inline void Rect::SetSize(Vec2 const size)
{
	mSize = size;
}

inline Vec2 const &Rect::GetSize() const
{
	return mSize;
}

inline float Rect::Right() const
{
	return mOrigin.x + mSize.x;
}

inline float Rect::Left() const
{
	return mOrigin.x;
}

inline float Rect::Bottom() const
{
	return mOrigin.y + mSize.y;
}

inline float Rect::Top() const
{
	return mOrigin.y;
}

inline void Rect::Move(Vec2 offset)
{
	mOrigin += offset;
}

inline void Rect::Inflate(Vec2 amount)
{
	Vec2 half = amount / 2.0f;
	mOrigin -= half;
	mSize += half;
}

inline void Rect::Deflate(Vec2 amount)
{
	Vec2 half = amount / 2.0f;
	mOrigin += half;
	mSize -= half;
}

inline bool Rect::IsEmpty() const
{
	return GetWidth() == 0 || GetHeight() == 0;
}

inline Rect Rect::Intersect(Rect const &o) const
{
	float left = Max(Left(), o.Left());
	float top = Max(Top(), o.Top());
	float right = Min(Right(), o.Right());
	float bottom = Min(Bottom(), o.Bottom());
	float width = Max(0.0f, right - left);
	float height = Max(0.0f, bottom - top);
	return Rect(left, top, width, height);
}

inline Rect Rect::Union(Rect const &o) const
{
	float left = Min(Left(), o.Left());
	float top = Min(Top(), o.Top());
	float right = Max(Right(), o.Right());
	float bottom = Max(Bottom(), o.Bottom());
	float width = right - left;
	float height = bottom - top;
	return Rect(left, top, width, height);
}
