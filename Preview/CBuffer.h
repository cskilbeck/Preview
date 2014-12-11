//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct CBuffer
{
	using Map = std::unordered_map <string, CBuffer *>;

	struct Parameter
	{
		D3D11_SHADER_VARIABLE_DESC	Variable;
		D3D11_SHADER_TYPE_DESC		Type;
	};

	using ParamMap = std::unordered_map<char const *, Parameter *>;

	char const *Name;
	size_t		TotalSizeInBytes;
	ParamMap	Parameters;
	Ptr<byte>	Buffer;
	ID3D11Buffer *mConstantBuffer;

	HRESULT Create(ID3D11ShaderReflectionConstantBuffer *b);

	Parameter *GetParameter(char const *name)
	{
		ParamMap::iterator i = Parameters.find(name);
		return (i != Parameters.end()) ? i->second : null;
	}

	byte *GetAddressAndSizeOf(char const *name, size_t &size)
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

	byte *GetAddressOf(char const *name)
	{
		Parameter *p = GetParameter(name);
		return (p != null) ? Buffer.get() + p->Variable.StartOffset : null;
	}
};

