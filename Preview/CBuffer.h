//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct ConstantBuffer
{
	ConstantBuffer();
	~ConstantBuffer();

	struct Parameter
	{
		D3D11_SHADER_VARIABLE_DESC	Variable;
		D3D11_SHADER_TYPE_DESC		Type;
	};

	HRESULT		Create(ID3D11ShaderReflectionConstantBuffer *b);
	byte *		AddressAndSizeOf(String const &name, size_t &size);
	int			IndexOf(String const &name);
	byte *		AddressOf(String const &name);
	byte *		AddressOf(int index);
	uint		SizeOf(int index) const;
	uint		SizeOf(String const &name) const;
	size_t		SizeOf(int index);
	byte *		GetBuffer();
	void		Commit(ID3D11DeviceContext *context);

	template <typename T> void Set(char const *name, T const &value)
	{
		assert(sizeof(T) == SizeOf(name));
		byte *p = AddressOf(name);
		*((T *)p) = value;
	}

	template <typename T> void Set(uint index, T const &value)
	{
		assert(sizeof(T) == SizeOf(index));
		byte *p = AddressOf(name);
		*((T *)p) = value;
	}

	byte * operator [] (char const *name)
	{
		return AddressOf(name);
	}

	char const *		Name;
	uint32				TotalSizeInBytes;
	AlignedPtr<byte>	Buffer;
	DXPtr<ID3D11Buffer>	mConstantBuffer;

private:

	using ParamVec = Vector<Parameter>;
	using IntMap = std::map<String, int>;

	ParamVec	Parameters;
	IntMap		ParameterIDs;

	Parameter const *GetParameter(String const &name) const;
	Parameter *	GetParameter(String const &name);
};

//////////////////////////////////////////////////////////////////////

inline ConstantBuffer::Parameter *ConstantBuffer::GetParameter(String const &name)
{
	auto i = ParameterIDs.find(name);
	return (i != ParameterIDs.end()) ? &Parameters[i->second] : null;
}

//////////////////////////////////////////////////////////////////////

inline ConstantBuffer::Parameter const *ConstantBuffer::GetParameter(String const &name) const
{
	auto i = ParameterIDs.find(name);
	return (i != ParameterIDs.end()) ? &Parameters[i->second] : null;
}

//////////////////////////////////////////////////////////////////////

int ConstantBuffer::IndexOf(String const &name)
{
	auto i = ParameterIDs.find(name);
	return (i != ParameterIDs.end()) ? i->second : -1;
}

//////////////////////////////////////////////////////////////////////

inline byte *ConstantBuffer::AddressAndSizeOf(String const &name, size_t &size)
{
	byte *addr = null;
	Parameter *p = GetParameter(name);
	if(p != null)
	{
		size = p->Variable.Size;
		addr = Buffer.get() + p->Variable.StartOffset;
	}
	return addr;
}

//////////////////////////////////////////////////////////////////////

inline byte *ConstantBuffer::AddressOf(String const &name)
{
	Parameter *p = GetParameter(name);
	return (p != null) ? Buffer.get() + p->Variable.StartOffset : null;
}

//////////////////////////////////////////////////////////////////////

inline byte *ConstantBuffer::AddressOf(int index)
{
	assert(index >= 0 && index < (int)Parameters.size());
	return Buffer.get() + Parameters[index].Variable.StartOffset;
}

//////////////////////////////////////////////////////////////////////

inline uint ConstantBuffer::SizeOf(String const &name) const
{
	Parameter const *p = GetParameter(name);
	return (p != null) ? p->Variable.Size : 0;
}

//////////////////////////////////////////////////////////////////////

inline uint ConstantBuffer::SizeOf(int index) const
{
	assert(index >= 0 && index < (int)Parameters.size());
	return Parameters[index].Variable.Size;
}

//////////////////////////////////////////////////////////////////////

inline byte *ConstantBuffer::GetBuffer()
{
	return Buffer.get();
}

