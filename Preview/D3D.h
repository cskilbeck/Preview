//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#if defined(DEBUG)
#define DX(x) { HRESULT hr = (x); if(FAILED(hr)) { TRACE(#x" failed: %08x\n", hr); assert(false); return hr; } }
#define DXV(x) { HRESULT hr = (x); if(FAILED(hr)) { TRACE(#x" failed: %08x\n", hr); assert(false); return; } }
#define DXB(x) { HRESULT hr = (x); if(FAILED(hr)) { TRACE(#x" failed: %08x\n", hr); assert(false); return false; } }
#else
#define DX(x) { HRESULT hr = (x); if(FAILED(hr)) return hr; }
#define DXV(x) { HRESULT hr = (x); if(FAILED(hr)) return; }
#define DXB(x) { HRESULT hr = (x); if(FAILED(hr)) return false; }
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
	DXPtr(T *init = nullptr) : p(init)
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

	ULONG Release()
	{
		ULONG r = 0;
		if(p != nullptr)
		{
			r = p->Release();
			p = nullptr;
		}
		return r;
	}

	T *p;
	void *leak;
};