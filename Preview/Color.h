//////////////////////////////////////////////////////////////////////

struct Color
{
	uint32 mColor;

	enum
	{
		kAlphaOffset = 24,
		kRedOffset = 16,
		kGreenOffset = 8,
		kBlueOffset = 0
	};

	enum
	{
		kAlphaMask	= 0xff << kAlphaOffset,
		kRedMask	= 0xff << kRedOffset,
		kGreenMask	= 0xff << kGreenOffset,
		kBlueMask	= 0xff << kBlueOffset,
	};

	Color()
	{
	}

	Color(uint32 c)
		: mColor(c)
	{
	}

	operator uint32()
	{
		return mColor;
	}

	Color(Color const &other)
		: mColor(other.mColor)
	{
	}

	Color(byte a, byte r, byte g, byte b)
	{
		mColor =	(uint32)a << kAlphaOffset |
					(uint32)r << kRedOffset |
					(uint32)g << kGreenOffset |
					(uint32)b << kBlueOffset;
	}
	
	Color(byte r, byte g, byte b)
	{
		mColor =	0xff		<< kAlphaOffset |
					(uint32)r	<< kRedOffset |
					(uint32)g	<< kGreenOffset |
					(uint32)b	<< kBlueOffset;
	}
	
	void GetFloats(float *f)
	{
		f[0] = Red() / 255.0f;
		f[1] = Green() / 255.0f;
		f[2] = Blue() / 255.0f;
		f[3] = Alpha() / 255.0f;
	}

	Color &operator = (Color o)
	{
		mColor = o.mColor;
		return *this;
	}

	Color &operator = (uint32 c)
	{
		mColor = c;
		return *this;
	}

	Color &operator = (int c)
	{
		mColor = (uint32)c;
		return *this;
	}

	uint32 Alpha() const	{ 	return (mColor >> kAlphaOffset)	& 0xff;	}
	uint32 Red() const		{	return (mColor >> kRedOffset)	& 0xff;	}
	uint32 Green() const	{	return (mColor >> kGreenOffset)	& 0xff;	}
	uint32 Blue() const		{	return (mColor >> kBlueOffset)	& 0xff;	}

	void SetAlpha(byte a)	{	mColor = (mColor & kAlphaMask)	| ((uint32)a << kAlphaOffset);	}
	void SetRed(byte r)		{	mColor = (mColor & kRedMask)	| ((uint32)r << kRedOffset);	}
	void SetGreen(byte g)	{	mColor = (mColor & kGreenMask)	| ((uint32)g << kGreenOffset);	}
	void SetBlue(byte b)	{	mColor = (mColor & kBlueMask)	| ((uint32)b << kBlueOffset);	}

	inline Color Lerp(Color &other, byte lerp)		// 0 = this, 255 = other, 128 = 50:50
	{
		int inv = 255 - lerp;
		return Color(	(Alpha()	* lerp + other.Alpha()	* inv) >> 8,
						(Red()		* lerp + other.Red()	* inv) >> 8,
						(Green()	* lerp + other.Green()	* inv) >> 8,
						(Blue()		* lerp + other.Blue()	* inv) >> 8);
	}

	Color operator * (Color const &o)
	{
		return Color(	Alpha()	* o.Alpha()	>> 8,
						Red()	* o.Red()	>> 8,
						Green()	* o.Green()	>> 8,
						Blue()	* o.Blue()	>> 8 );
	}

	enum : uint32
	{
		AliceBlue = 0xFFF0F8FF,
		AntiqueWhite = 0xFFFAEBD7,
		Aqua = 0xFF00FFFF,
		Aquamarine = 0xFF7FFFD4,
		Azure = 0xFFF0FFFF,
		Beige = 0xFFF5F5DC,
		Bisque = 0xFFFFE4C4,
		Black = 0xFF000000,
		BlanchedAlmond = 0xFFFFEBCD,
		BrightBlue = 0xFF0000FF,
		BrightGreen = 0xFF008000,
		BrightRed = 0xFFFF0000,
		BlueViolet = 0xFF8A2BE2,
		Brown = 0xFFA52A2A,
		BurlyWood = 0xFFDEB887,
		CadetBlue = 0xFF5F9EA0,
		Chartreuse = 0xFF7FFF00,
		Chocolate = 0xFFD2691E,
		Coral = 0xFFFF7F50,
		CornflowerBlue = 0xFF6495ED,
		Cornsilk = 0xFFFFF8DC,
		Crimson = 0xFFDC143C,
		Cyan = 0xFF00FFFF,
		DarkBlue = 0xFF00008B,
		DarkCyan = 0xFF008B8B,
		DarkGoldenrod = 0xFFB8860B,
		DarkGray = 0xFFA9A9A9,
		DarkGreen = 0xFF006400,
		DarkKhaki = 0xFFBDB76B,
		DarkMagenta = 0xFF8B008B,
		DarkOliveGreen = 0xFF556B2F,
		DarkOrange = 0xFFFF8C00,
		DarkOrchid = 0xFF9932CC,
		DarkRed = 0xFF8B0000,
		DarkSalmon = 0xFFE9967A,
		DarkSeaGreen = 0xFF8FBC8F,
		DarkSlateBlue = 0xFF483D8B,
		DarkSlateGray = 0xFF2F4F4F,
		DarkTurquoise = 0xFF00CED1,
		DarkViolet = 0xFF9400D3,
		DeepPink = 0xFFFF1493,
		DeepSkyBlue = 0xFF00BFFF,
		DimGray = 0xFF696969,
		DodgerBlue = 0xFF1E90FF,
		Firebrick = 0xFFB22222,
		FloralWhite = 0xFFFFFAF0,
		ForestGreen = 0xFF228B22,
		Fuchsia = 0xFFFF00FF,
		Gainsboro = 0xFFDCDCDC,
		GhostWhite = 0xFFF8F8FF,
		Gold = 0xFFFFD700,
		Goldenrod = 0xFFDAA520,
		Gray = 0xFF808080,
		GreenYellow = 0xFFADFF2F,
		Honeydew = 0xFFF0FFF0,
		HotPink = 0xFFFF69B4,
		IndianRed = 0xFFCD5C5C,
		Indigo = 0xFF4B0082,
		Ivory = 0xFFFFFFF0,
		Khaki = 0xFFF0E68C,
		Lavender = 0xFFE6E6FA,
		LavenderBlush = 0xFFFFF0F5,
		LawnGreen = 0xFF7CFC00,
		LemonChiffon = 0xFFFFFACD,
		LightBlue = 0xFFADD8E6,
		LightCoral = 0xFFF08080,
		LightCyan = 0xFFE0FFFF,
		LightGoldenrodYellow = 0xFFFAFAD2,
		LightGreen = 0xFF90EE90,
		LightGray = 0xFFD3D3D3,
		LightPink = 0xFFFFB6C1,
		LightSalmon = 0xFFFFA07A,
		LightSeaGreen = 0xFF20B2AA,
		LightSkyBlue = 0xFF87CEFA,
		LightSlateGray = 0xFF778899,
		LightSteelBlue = 0xFFB0C4DE,
		LightYellow = 0xFFFFFFE0,
		Lime = 0xFF00FF00,
		LimeGreen = 0xFF32CD32,
		Linen = 0xFFFAF0E6,
		Magenta = 0xFFFF00FF,
		Maroon = 0xFF800000,
		MediumAquamarine = 0xFF66CDAA,
		MediumBlue = 0xFF0000CD,
		MediumOrchid = 0xFFBA55D3,
		MediumPurple = 0xFF9370DB,
		MediumSeaGreen = 0xFF3CB371,
		MediumSlateBlue = 0xFF7B68EE,
		MediumSpringGreen = 0xFF00FA9A,
		MediumTurquoise = 0xFF48D1CC,
		MediumVioletRed = 0xFFC71585,
		MidnightBlue = 0xFF191970,
		MintCream = 0xFFF5FFFA,
		MistyRose = 0xFFFFE4E1,
		Moccasin = 0xFFFFE4B5,
		NavajoWhite = 0xFFFFDEAD,
		Navy = 0xFF000080,
		OldLace = 0xFFFDF5E6,
		Olive = 0xFF808000,
		OliveDrab = 0xFF6B8E23,
		Orange = 0xFFFFA500,
		OrangeRed = 0xFFFF4500,
		Orchid = 0xFFDA70D6,
		PaleGoldenrod = 0xFFEEE8AA,
		PaleGreen = 0xFF98FB98,
		PaleTurquoise = 0xFFAFEEEE,
		PaleVioletRed = 0xFFDB7093,
		PapayaWhip = 0xFFFFEFD5,
		PeachPuff = 0xFFFFDAB9,
		Peru = 0xFFCD853F,
		Pink = 0xFFFFC0CB,
		Plum = 0xFFDDA0DD,
		PowderBlue = 0xFFB0E0E6,
		Purple = 0xFF800080,
		RosyBrown = 0xFFBC8F8F,
		RoyalBlue = 0xFF4169E1,
		SaddleBrown = 0xFF8B4513,
		Salmon = 0xFFFA8072,
		SandyBrown = 0xFFF4A460,
		SeaGreen = 0xFF2E8B57,
		SeaShell = 0xFFFFF5EE,
		Sienna = 0xFFA0522D,
		Silver = 0xFFC0C0C0,
		SkyBlue = 0xFF87CEEB,
		SlateBlue = 0xFF6A5ACD,
		SlateGray = 0xFF708090,
		Snow = 0xFFFFFAFA,
		SpringGreen = 0xFF00FF7F,
		SteelBlue = 0xFF4682B4,
		Tan = 0xFFD2B48C,
		Teal = 0xFF008080,
		Thistle = 0xFFD8BFD8,
		Tomato = 0xFFFF6347,
		Transparent = 0x00000000,
		Turquoise = 0xFF40E0D0,
		Violet = 0xFFEE82EE,
		Wheat = 0xFFF5DEB3,
		White = 0xFFFFFFFF,
		WhiteSmoke = 0xFFF5F5F5,
		Yellow = 0xFFFFFF00,
		YellowGreen = 0xFF9ACD32
	};

};
