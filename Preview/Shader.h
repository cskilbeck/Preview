//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Shader
{
	vector<ID3D11Buffer *> mBuffers;
	CBuffer::Map mConstantBuffers;

	void DeleteConstantBuffers();
	HRESULT CreateConstantBuffers(void const *blob, size_t size);

	CBuffer *GetConstantBuffer(char const *name)
	{
		CBuffer::Map::const_iterator i = mConstantBuffers.find(name);
		return (i != mConstantBuffers.end()) ? i->second : null;
	}
};

