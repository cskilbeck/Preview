//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

using Matrix = DirectX::XMMATRIX;

//////////////////////////////////////////////////////////////////////

class HRException: public std::exception
{
public:
	HRException(HRESULT res) : hr(res) {}
	HRESULT hr;
};

//////////////////////////////////////////////////////////////////////

//#define DXTRACE TRACE
#define DXTRACE if (true) {} else

#if defined(DEBUG)
#define dx(hr, x)		\
	hr = (x);			\
	if(FAILED(hr))		\
	{					\
		TRACE(TEXT(#x) TEXT(" failed: %08x\n"), hr);	\
		assert(false);	\
		return hr;	\
	}					\
	else				\
	{					\
		DXTRACE(TEXT(#x) TEXT( " ok\n"));		\
	}					\

#define DX(x) 			\
	__hr = (x);			\
	if(FAILED(__hr))	\
	{					\
		TRACE(TEXT(#x) TEXT(" failed: %08x\n"), __hr);	\
		assert(false);	\
		return __hr;	\
	}					\
	else				\
	{					\
		DXTRACE(TEXT(#x) TEXT( " ok\n"));		\
	}					\

#define DXV(x) 			\
	__hr = (x);			\
	if(FAILED(__hr))	\
	{					\
		TRACE(TEXT(#x) TEXT(" failed: %08x\n"), __hr);	\
		assert(false);	\
		return;			\
	}					\
	else				\
	{					\
		DXTRACE(TEXT(#x) TEXT( " ok\n"));		\
	}					\

#define DXB(x) 			\
	__hr = (x);			\
	if(FAILED(__hr))	\
	{					\
		TRACE(TEXT(#x) TEXT(" failed: %08x\n"), __hr);	\
		assert(false);	\
		return false;	\
	}					\
	else				\
	{					\
		DXTRACE(TEXT(#x) TEXT( " ok\n"));		\
	}					\

#define DXZ(x) 			\
	__hr = (x);			\
	if(FAILED(__hr))	\
	{					\
		TRACE(TEXT(#x) TEXT(" failed: %08x\n"), __hr);	\
		assert(false);	\
		return 0;		\
	}					\
	else				\
	{					\
		DXTRACE(TEXT(#x) TEXT( " ok\n"));		\
	}					\

#define DXT(x) 			\
	__hr = (x);			\
	if(FAILED(__hr))	\
	{					\
		TRACE(TEXT(#x) TEXT(" failed: %08x\n"), __hr);	\
		assert(false);	\
		throw HRException(__hr);\
	}					\
	else				\
	{					\
		DXTRACE(TEXT(#x) TEXT( " ok\n"));		\
	}					\

#else
#define DX(x) __hr = (x); if(FAILED(__hr)) return __hr;
#define DXV(x) __hr = (x); if(FAILED(__hr)) return;
#define DXB(x) __hr = (x); if(FAILED(__hr)) return false;
#define DXZ(x) __hr = (x); if(FAILED(__hr)) return 0;
#define DXT(x) __hr = (x); if(FAILED(__hr)) throw HRException(__hr);
#endif

//////////////////////////////////////////////////////////////////////

inline void SetDebugName(ID3D11DeviceChild* child, tchar const *name)
{
	if(child != null && name != null)
	{
		child->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)_tcslen(name), name);
	}
}

//////////////////////////////////////////////////////////////////////

template<typename T> struct DXPtr
{
	DXPtr(T *init = null) : p(init)
	{
	}

	DXPtr(DXPtr const &ptr) : p(ptr.p)
	{
		p->AddRef();
	}

	DXPtr(DXPtr &&ptr) : p(null)
	{
		std::swap(p, ptr.p);
	}

	DXPtr &operator=(DXPtr const &ptr)
	{
		Release();
		p = ptr.p;
		p->AddRef();
		return *this;
	}

	DXPtr &operator=(DXPtr&& ptr)
	{
		std::swap(p, ptr.p);
		return *this;
	}

	~DXPtr()
	{
		Release();
	}

	bool IsNull() const
	{
		return p == null;
	}

	T **operator &()
	{
		return &p;
	}

	T *operator->() const
	{
		return p;
	}

	T &operator *() const
	{
		return *p;
	}

	operator T *()
	{
		return p;
	}

	bool operator == (nullptr_t) const
	{
		return p == null;
	}

	bool operator != (nullptr_t) const
	{
		return p != null;
	}

	T *get() const
	{
		return p;
	}

	LONG Release()
	{
		LONG r = 0;
		if(p != null)
		{
			r = p->Release();
			p = null;
		}
		return r;
	}

	T *p;
};

//////////////////////////////////////////////////////////////////////

extern __declspec(selectany) HRESULT __hr;
extern DXPtr<ID3D11Device> gDevice;

//////////////////////////////////////////////////////////////////////

enum InputType
{
	Float_input = 0,
	Int_input = 1,
	Short_input = 2,
	Byte_input = 3
};

struct field_definition
{
	char const *semanticName;
	int index;
	InputType type;
	int elementCount;
	DWORD format;
};

#define struct_def_vert(name) __declspec(align(4)) struct name {
#define struct_element(type, size, name, meaning, index, format) HLSL::type##size name; 
#define struct_end_def() static const field_definition fields[]; };

#define array_def_vert(name) const __declspec(selectany) field_definition name::fields[] = {
#define array_element(type, size, name, meaning, index, format) { #meaning, index, type##_input, size, format },
#define array_end_def() };

#define vert_GenerateStruct(T) T(struct_def_vert, struct_element, struct_end_def)
#define vert_GenerateFields(T) T(array_def_vert, array_element, array_end_def); 

namespace HLSL
{
#pragma pack(push, 4)

#pragma push_macro("join2")
#pragma push_macro("join3")
#pragma push_macro("join4")
#pragma push_macro("def_vec")
#pragma push_macro("vec_template")

#define join2(x, y) x ## y
#define join3(x, y, z) join2(x, y) ## z
#define join4(x, y, z, w) join3(x, y, z) ## w

	using Byte = byte;
	using Int = int32;
	using UInt = uint32;
	using DWord = uint32;
	using Float = float;
	using Void = void;		// typeless?

	enum
	{
		X = 0,
		Y = 1,
		Z = 2,
		W = 3
	};

#define vec_template(N, ...)									\
	template <typename t> struct Vec##N							\
	{															\
		union													\
		{														\
			t m[N];												\
			struct												\
			{													\
				t __VA_ARGS__ ;									\
			};													\
		};														\
		template <typename u> Vec##N &operator = (u const &src) \
		{														\
			assert(sizeof(m) == sizeof(u));						\
			memcpy(m, &src, sizeof(m));							\
			return *this;										\
		}														\
	};

	vec_template(1, x)
	vec_template(2, x, y)
	vec_template(3, x, y, z)
	vec_template(4, x, y, z, w)

	template <> struct Vec4<byte>
	{
		union
		{
			byte m[4];
			uint32 c;
			struct
			{
				byte x, y, z, w;
			};
		};

		Vec4()
		{
		}

		Vec4(uint32 cc) : c(cc)
		{
		}

		Vec4<byte> &operator = (uint32 cc)
		{
			c = cc;
			return *this;
		}

		operator uint32 () const
		{
			return c;
		}
	};

	template <> struct Vec4<float>
	{
		union
		{
			float m[4];
			struct
			{
				float x, y, z, w;
			};
			__m128 m128;
		};

		Vec4()
		{
		}

		Vec4(Vec4<float> const &o)
		{
			m128 = o.m128;
		}

		Vec4(__m128 v)
		{
			m128 = v;
		}

		Vec4<float> &operator = (__m128 v)
		{
			m128 = v;
			return *this;
		}

		operator __m128() const
		{
			return m128;
		}
	};

#define def_vec(T)							\
	using join2(T, 1) = join2(Vec, 1)<T>;	\
	using join2(T, 2) = join2(Vec, 2)<T>;	\
	using join2(T, 3) = join2(Vec, 3)<T>;	\
	using join2(T, 4) = join2(Vec, 4)<T>;

	def_vec(Byte)
	def_vec(Int)
	def_vec(UInt)
	def_vec(DWord)
	def_vec(Float)

#pragma pop_macro("join2")
#pragma pop_macro("join3")
#pragma pop_macro("join4")
#pragma pop_macro("def_vec")
#pragma pop_macro("vec_template")
	
#pragma pack(pop)
}

HRESULT CreateInputLayout(field_definition const definitions[], int elements, void const *blob, size_t size, ID3D11InputLayout **vertexLayout);
