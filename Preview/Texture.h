//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Texture
{
	static Texture *Load(char const *name);
	static Texture *Create(int width, int height, Color color);

	int Width() const { return mSize.w; }
	int Height() const { return mSize.h; }
	bool IsValid() const { return impl != null; }
	Size2D const &GetSize() const { return mSize; }

	void Activate();

	Size2D	mSize;
	string	mName;

private:

	Texture();
	Texture(char const *name);
	Texture(int w, int h, Color color);
	~Texture();

	struct TextureImpl;
	friend struct TextureImpl;
	TextureImpl *impl;
};

//////////////////////////////////////////////////////////////////////

