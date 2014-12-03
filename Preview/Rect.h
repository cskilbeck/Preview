
struct Rect2D
{
	Point2D	mTopLeft;
	Size2D	mSize;

	Rect2D()
	{
	}

	Rect2D(RECT const &rc)
		: mTopLeft(rc.left, rc.top)
		, mSize(rc.right - rc.left, rc.bottom - rc.top)
	{
	}

	operator RECT() const
	{
		RECT r = { mTopLeft.x, mTopLeft.y, mTopLeft.x + mSize.w, mTopLeft.y + mSize.h };
		return r;
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

	bool Contains(Point2D p) const
	{
		return p.x >= mTopLeft.x && p.x < mTopLeft.x + mSize.w && p.y >= mTopLeft.y && p.y < mTopLeft.y + mSize.h;
	}
};
