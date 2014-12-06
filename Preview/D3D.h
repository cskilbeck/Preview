//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

using Matrix = DirectX::XMMATRIX;

extern HRESULT __hr;

#if defined(DEBUG)
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
		TRACE(TEXT(#x) TEXT( " ok\n"));		\
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
		TRACE(TEXT(#x) TEXT( " ok\n"));		\
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
		TRACE(TEXT(#x) TEXT( " ok\n"));		\
	}					\

#else
#define DX(x) __hr = (x); if(FAILED(__hr)) return __hr;
#define DXV(x) __hr = (x); if(FAILED(__hr)) return;
#define DXB(x) __hr = (x); if(FAILED(__hr)) return false;
#endif

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

	DXPtr(DXPtr &&ptr) : p(nullptr)
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
		if(p != nullptr)
		{
			r = p->Release();
			p = nullptr;
		}
		return r;
	}

	T *p;
};