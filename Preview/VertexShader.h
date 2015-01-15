//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct VertexShader: Shader
{
	DXPtr<ID3D11VertexShader>	vertexShader;
	DXPtr<ID3D11InputLayout>	vertexLayout;

	virtual ~VertexShader() override;
	virtual HRESULT Destroy() override;

	HRESULT Create(void const *blob, size_t size, D3D11_INPUT_ELEMENT_DESC *inputDesc, int numElements);
	void Activate(ID3D11DeviceContext *context);
	void LoadConstants(ID3D11DeviceContext *context);
};

//////////////////////////////////////////////////////////////////////

template <typename T> struct VertexBuffer
{
	HRESULT Create(int vertCount)
	{
		vertexCount = vertCount;
		TRACE("Vertex Size: %d\n", sizeof(T));
		memBuffer.reset(new T[vertexCount]);
		CD3D11_BUFFER_DESC bd(sizeof(T) * vertexCount, D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA InitData = { (void *)memBuffer.get(), 0, 0 };
		DX(gDevice->CreateBuffer(&bd, &InitData, &buffer));
		return S_OK;
	}

	void Destroy()
	{
		buffer.Release();
		memBuffer.reset();
	}

	void Commit(ID3D11DeviceContext *context)
	{
		context->UpdateSubresource(buffer, 0, null, memBuffer.get(), 0, 0);
	}

	void Activate(ID3D11DeviceContext *context)
	{
		UINT stride = sizeof(T);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	}

	T *GetBuffer()
	{
		return memBuffer.get();
	}

	uint VertexCount() const
	{
		return vertexCount;
	}

	size_t VertexSize() const
	{
		return sizeof(T);
	}

	uint vertexCount;
	DXPtr<ID3D11Buffer> buffer;
	Ptr<T> memBuffer;
};
