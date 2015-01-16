//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct VertexShader: Shader
{
	DXPtr<ID3D11VertexShader>	vertexShader;

	virtual ~VertexShader() override;
	virtual HRESULT Destroy() override;

	HRESULT Create(Resource const &blob, field_definition const inputDesc[], int numElements);
	void Activate(ID3D11DeviceContext *context);
	void LoadConstants(ID3D11DeviceContext *context);
};

//////////////////////////////////////////////////////////////////////

struct VertexBuffer
{
	template <typename T> HRESULT Create(int vertCount, Resource const &vsBlob)
	{
		vertexCount = vertCount;
		vertexSize = sizeof(T);
		memBuffer.reset(new byte[vertexCount * vertexSize]);
		CD3D11_BUFFER_DESC bd(sizeof(T) * vertexCount, D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA InitData = { (void *)memBuffer.get(), 0, 0 };
		DX(gDevice->CreateBuffer(&bd, &InitData, &buffer));
		DX(CreateInputLayout(T::fields, _countof(T::fields), vsBlob, vsBlob.Size(), &vertexLayout));
		return S_OK;
	}

	void Destroy()
	{
		vertexLayout.Release();
		buffer.Release();
		memBuffer.reset();
	}

	void Commit(ID3D11DeviceContext *context)
	{
		context->UpdateSubresource(buffer, 0, null, memBuffer.get(), 0, 0);
	}

	void Activate(ID3D11DeviceContext *context)
	{
		UINT stride = vertexSize;
		UINT offset = 0;
		context->IASetInputLayout(vertexLayout);
		context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	}

	byte *GetBuffer()
	{
		return memBuffer.get();
	}

	uint VertexCount() const
	{
		return vertexCount;
	}

	uint VertexSize() const
	{
		return vertexSize;
	}

	uint vertexCount;
	uint vertexSize;
	DXPtr<ID3D11Buffer> buffer;
	DXPtr<ID3D11InputLayout> vertexLayout;
	Ptr<byte> memBuffer;
};

