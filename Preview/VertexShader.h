//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct VertexShader: Shader
{
	DXPtr<ID3D11VertexShader>	vertexShader;
	DXPtr<ID3D11InputLayout>	vertexLayout;

	virtual ~VertexShader() override;
	virtual HRESULT Destroy() override;

	HRESULT CreateInputLayout(void const *blob, size_t size);
	HRESULT Create(void const *blob, size_t size);
	void Activate(ID3D11DeviceContext *context);
	void LoadConstants(ID3D11DeviceContext *context);

	uint vertexSize;
};

//////////////////////////////////////////////////////////////////////

template <typename T> struct VertexBuffer
{
	HRESULT Create(int vertexCount, VertexShader &shader)
	{
		memBuffer.reset(new T[vertexCount]);
		CD3D11_BUFFER_DESC bd(sizeof(T), D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA InitData = { (void *)memBuffer.get(), 0, 0 };
		DX(gDevice->CreateBuffer(&bd, &InitData, &buffer));
		return S_OK;
	}

	HRESULT Update(ID3D11DeviceContext *context)
	{
		context->UpdateSubresource(buffer, 0, null, memBuffer.get(), 0, 0);
		return S_OK;
	}

	void Activate(ID3D11DeviceContext *context)
	{
		UINT strides[] = { sizeof(T) };
		UINT offsets[] = { 0 };
		context->IASetVertexBuffers(0, 1, &buffer, strides, offsets);
	}

	operator T * ()
	{
		return memBuffer.get();
	}

	uint vertexSize;
	DXPtr<ID3D11Buffer> buffer;
	Ptr<T> memBuffer;
};
