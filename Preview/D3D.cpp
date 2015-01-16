//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

#pragma comment(lib, "d3dcompiler.lib")

//////////////////////////////////////////////////////////////////////

DXPtr<ID3D11Device> gDevice;

//////////////////////////////////////////////////////////////////////

HRESULT CreateInputLayout(field_definition const definitions[], int elements, void const *blob, size_t size, ID3D11InputLayout **vertexLayout)
{
	Ptr<D3D11_INPUT_ELEMENT_DESC> ied(new D3D11_INPUT_ELEMENT_DESC[elements]);
	D3D11_INPUT_ELEMENT_DESC *d = ied.get();
	field_definition const *f = definitions;
	for(int i = 0; i < elements; ++i, ++d, ++f)
	{
		d->Format = (DXGI_FORMAT)f->format;
		d->SemanticIndex = f->index;
		d->SemanticName = f->semanticName;
		d->AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		d->InputSlot = 0;
		d->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		d->InstanceDataStepRate = 0;
	}
	DX(gDevice->CreateInputLayout(ied.get(), elements, blob, size, vertexLayout));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////
