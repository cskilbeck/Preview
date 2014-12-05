//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

using Matrix = DirectX::XMMATRIX;

extern HRESULT hr;

#if defined(DEBUG)
#define DX(x) 			\
	hr = (x);			\
	if(FAILED(hr))		\
	{					\
		TRACE(TEXT(#x) TEXT(" failed: %08x\n"), hr);	\
		assert(false);	\
		return hr;		\
	}					\
	else				\
	{					\
		TRACE(TEXT(#x) TEXT( " ok\n"));		\
	}					\
	hr;

#define DXV(x) 			\
	hr = (x);			\
	if(FAILED(hr))		\
	{					\
		TRACE(TEXT(#x) TEXT(" failed: %08x\n"), hr);	\
		assert(false);	\
		return;			\
	}					\
	else				\
	{					\
		TRACE(TEXT(#x) TEXT( " ok\n"));		\
	}					\
	hr;

#define DXB(x) 			\
	hr = (x);			\
	if(FAILED(hr))		\
	{					\
		TRACE(TEXT(#x) TEXT(" failed: %08x\n"), hr);	\
		assert(false);	\
		return false;	\
	}					\
	else				\
	{					\
		TRACE(TEXT(#x) TEXT( " ok\n"));		\
	}					\
	hr;

#else
#define DX(x) hr = (x); if(FAILED(hr)) return hr;
#define DXV(x) hr = (x); if(FAILED(hr)) return;
#define DXB(x) hr = (x); if(FAILED(hr)) return false;
#endif

inline void SetDebugName(ID3D11DeviceChild* child, tchar const *name)
{
	if(child != null && name != null)
	{
		child->SetPrivateData(WKPDID_D3DDebugObjectName, _tcslen(name), name);
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
		reset();
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