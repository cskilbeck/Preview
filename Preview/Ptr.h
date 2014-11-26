#pragma once

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

template<typename T> struct Ptr
{
	Ptr(T *ptr = null) : p(ptr)
	{
	}

	Ptr(Ptr const &ptr) : p(ptr.p)
	{
		p->AddRef();
	}

	Ptr(Ptr &&ptr) : p(null)
	{
		std::swap(p, ptr.p);
	}

	~Ptr()
	{
		Release();
	}

	Ptr &operator =(Ptr const &ptr)
	{
		p = ptr.p;
		p->AddRef();
		return *this;
	}

	Ptr &operator =(Ptr&& ptr)
	{
		std::swap(p, ptr.p);
		return *this;
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

	T *get() const
	{
		return p;
	}

	bool IsNull() const
	{
		return p == null;
	}

	void Release()
	{
		if(!IsNull())
		{
			p->Release();
			p = null;
		}
	}

private:

	T *p;
};