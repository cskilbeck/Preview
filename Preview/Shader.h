//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Shader
{
	//////////////////////////////////////////////////////////////////////

	using IntMap = std::unordered_map<tstring, int>;

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

	int GetTextureIndex(tstring const &name) const;
	int GetSamplerIndex(tstring const &name) const;
	int GetConstantBufferIndex(tstring const &name) const;

	void SetTexture(int index, ID3D11ShaderResourceView *t);
	void SetTexture(tstring const &name, ID3D11ShaderResourceView *t);

	void SetSampler(int index, ID3D11SamplerState *s);
	void SetSampler(tstring const &name, ID3D11SamplerState *s);

	uint GetConstantBufferCount() const;
	ConstantBuffer *GetCB(tstring const &name);
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

static inline int GetIndex(tstring const &name, Shader::IntMap const &map)
{
	auto i = map.find(name);
	return (i == map.end()) ? -1 : i->second;
}

//////////////////////////////////////////////////////////////////////

inline int Shader::GetTextureIndex(tstring const &name) const
{
	return GetIndex(name, mTextureIDs);
}

//////////////////////////////////////////////////////////////////////

inline int Shader::GetSamplerIndex(tstring const &name) const
{
	return GetIndex(name, mSamplerIDs);
}

//////////////////////////////////////////////////////////////////////

inline int Shader::GetConstantBufferIndex(tstring const &name) const
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

inline void Shader::SetTexture(tstring const &name, ID3D11ShaderResourceView *t)
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

inline void Shader::SetSampler(tstring const &name, ID3D11SamplerState *s)
{
	SetSampler(GetSamplerIndex(name), s);
}

//////////////////////////////////////////////////////////////////////

inline uint Shader::GetConstantBufferCount() const
{
	return (uint)mConstantBuffers.size();
}

//////////////////////////////////////////////////////////////////////

inline ConstantBuffer *Shader::GetCB(tstring const &name)
{
	int id = GetConstantBufferIndex(name);
	return (id >= 0) ? mConstantBuffers[id] : null;
}

//////////////////////////////////////////////////////////////////////

inline ConstantBuffer *Shader::GetConstantBuffer(int index)
{
	return mConstantBuffers[index];
}

