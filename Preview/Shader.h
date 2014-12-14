//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Shader
{
	//////////////////////////////////////////////////////////////////////

	using IntMap = std::unordered_map<string, int32>;

	//////////////////////////////////////////////////////////////////////

	uint								mStartIndex;
	vector<ID3D11Buffer *>				mBuffers;

	DXPtr<ID3D11ShaderReflection>		mReflector;
	D3D11_SHADER_DESC					mShaderDesc;

	vector<ID3D11SamplerState *>		mSamplers;	// client fills these in
	vector<ID3D11ShaderResourceView *>	mTextures;

	vector<ConstantBuffer *>			mConstantBuffers;

	IntMap								mConstBufferIDs;
	IntMap								mSamplerIDs;
	IntMap								mTextureIDs;

	//////////////////////////////////////////////////////////////////////

	Shader();
	virtual ~Shader();

	int GetTextureIndex(string const &name) const;
	int GetSamplerIndex(string const &name) const;
	int GetConstantBufferIndex(string const &name) const;

	void SetTexture(int index, ID3D11ShaderResourceView *t);
	void SetTexture(string const &name, ID3D11ShaderResourceView *t);

	void SetSampler(int index, ID3D11SamplerState *s);
	void SetSampler(string const &name, ID3D11SamplerState *s);

	uint GetConstantBufferCount() const;
	ConstantBuffer *GetCB(string const &name);
	ConstantBuffer *GetConstantBuffer(int index);

	HRESULT CreateConstantBuffer(D3D11_SHADER_INPUT_BIND_DESC desc);
	HRESULT CreateTextureBinding(D3D11_SHADER_INPUT_BIND_DESC desc);
	HRESULT CreateSamplerBinding(D3D11_SHADER_INPUT_BIND_DESC desc);

	void DeleteConstantBuffers();

	HRESULT Create(void const *blob, size_t size);
	virtual HRESULT Destroy();

	HRESULT CreateBindings();
	HRESULT CreateBinding(D3D11_SHADER_INPUT_BIND_DESC desc);
	void DestroyBindings();

};

//////////////////////////////////////////////////////////////////////

static inline int GetIndex(string const &name, Shader::IntMap const &map)
{
	auto i = map.find(name);
	return (i == map.end()) ? -1 : i->second;
}

//////////////////////////////////////////////////////////////////////

inline int Shader::GetTextureIndex(string const &name) const
{
	return GetIndex(name, mTextureIDs);
}

//////////////////////////////////////////////////////////////////////

inline int Shader::GetSamplerIndex(string const &name) const
{
	return GetIndex(name, mSamplerIDs);
}

//////////////////////////////////////////////////////////////////////

inline int Shader::GetConstantBufferIndex(string const &name) const
{
	return GetIndex(name, mConstBufferIDs);
}

//////////////////////////////////////////////////////////////////////

inline void Shader::SetTexture(int index, ID3D11ShaderResourceView *t)
{
	if(index >= 0)
	{
		mTextures[index] = t;
	}
}

//////////////////////////////////////////////////////////////////////

inline void Shader::SetTexture(string const &name, ID3D11ShaderResourceView *t)
{
	SetTexture(GetTextureIndex(name), t);
}

//////////////////////////////////////////////////////////////////////

inline void Shader::SetSampler(int index, ID3D11SamplerState *s)
{
	if(index >= 0)
	{
		mSamplers[index] = s;
	}
}

//////////////////////////////////////////////////////////////////////

inline void Shader::SetSampler(string const &name, ID3D11SamplerState *s)
{
	SetSampler(GetSamplerIndex(name), s);
}

//////////////////////////////////////////////////////////////////////

inline uint Shader::GetConstantBufferCount() const
{
	return (uint)mConstantBuffers.size();
}

//////////////////////////////////////////////////////////////////////

inline ConstantBuffer *Shader::GetCB(string const &name)
{
	int id = GetConstantBufferIndex(name);
	return (id >= 0) ? mConstantBuffers[id] : null;
}

//////////////////////////////////////////////////////////////////////

inline ConstantBuffer *Shader::GetConstantBuffer(int index)
{
	return mConstantBuffers[index];
}

