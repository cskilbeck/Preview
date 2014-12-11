//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

HRESULT CBuffer::Create(ID3D11ShaderReflectionConstantBuffer *b)
{
	D3D11_SHADER_BUFFER_DESC sbDesc;
	b->GetDesc(&sbDesc);
	Name = sbDesc.Name;
	Parameters.clear();
	TotalSizeInBytes = sbDesc.Size;
	Buffer.reset(new byte[sbDesc.Size]);
	// Create a buffer
	CD3D11_BUFFER_DESC desc(sbDesc.Size, D3D11_BIND_CONSTANT_BUFFER);
	DX(gDevice->CreateBuffer(&desc, NULL, &mConstantBuffer));
	for(uint j = 0; j < sbDesc.Variables; ++j)
	{
		ID3D11ShaderReflectionVariable *var = b->GetVariableByIndex(j);
		ID3D11ShaderReflectionType *type = var->GetType();
		CBuffer::Parameter *cbVar = new CBuffer::Parameter();
		DX(var->GetDesc(&cbVar->Variable));
		DX(type->GetDesc(&cbVar->Type));
		Parameters[cbVar->Variable.Name] = cbVar;
	}
}

