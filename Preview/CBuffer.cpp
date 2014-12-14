//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

static char const *classNames[] =
{
	"scalar",
	"vector",
	"matrix_rows",
	"matrix_columns",
	"object",
	"struct",
	"interface_class",
	"interface_pointer"
};

//////////////////////////////////////////////////////////////////////

static bool isMatrix[] =
{
	false,	//"scalar",
	false,	//"vector",
	true,	//"matrix_rows",
	true,	//"matrix_columns",
	false,	//"object",
	false,	//"struct",
	false,	//"interface_class",
	false	//"interface_pointer"
};

//////////////////////////////////////////////////////////////////////

static char const *typeNames[] =
{
	"void",
	"bool",
	"int",
	"float",
	"string",
	"texture",
	"texture1d",
	"texture2d",
	"texture3d",
	"texturecube",
	"sampler",
	"pixelshader",
	"vertexshader",
	"uint",
	"uint8",
	"geometryshader",
	"rasterizer",
	"depthstencil",
	"blend",
	"buffer",
	"cbuffer",
	"tbuffer",
	"texture1darray",
	"texture2darray",
	"rendertargetview",
	"depthstencilview",
	"texture2dms",
	"texture2dmsarray",
	"texturecubearray",
	"hullshader",
	"domainshader",
	"interface_pointer",
	"computeshader",
	"double",
	"rwtexture1d",
	"rwtexture1darray",
	"rwtexture2d",
	"rwtexture2darray",
	"rwtexture3d",
	"rwbuffer",
	"byteaddress_buffer",
	"rwbyteaddress_buffer",
	"structured_buffer",
	"rwstructured_buffer",
	"append_structured_buffer",
	"consume_structured_buffer"
};

//////////////////////////////////////////////////////////////////////

ConstantBuffer::ConstantBuffer()
	: Name(null)
	, TotalSizeInBytes(0)
	, mConstantBuffer(null)
{
}

//////////////////////////////////////////////////////////////////////

ConstantBuffer::~ConstantBuffer()
{
	mConstantBuffer->Release();
}

//////////////////////////////////////////////////////////////////////

HRESULT ConstantBuffer::Create(ID3D11ShaderReflectionConstantBuffer *b)
{
	// get the details
	D3D11_SHADER_BUFFER_DESC sbDesc;
	b->GetDesc(&sbDesc);

	// fill them in
	Name = sbDesc.Name;
	Parameters.clear();
	TotalSizeInBytes = sbDesc.Size;
	Buffer.reset(new byte[sbDesc.Size]);

	// create the actual buffer
	CD3D11_BUFFER_DESC desc(sbDesc.Size, D3D11_BIND_CONSTANT_BUFFER);
	D3D11_SUBRESOURCE_DATA srd;
	srd.pSysMem = (void const *)Buffer.get();
	DX(gDevice->CreateBuffer(&desc, &srd, &mConstantBuffer));

	// get the details of the parameters
	for(uint j = 0; j < sbDesc.Variables; ++j)
	{
		ID3D11ShaderReflectionVariable *var = b->GetVariableByIndex(j);
		ID3D11ShaderReflectionType *type = var->GetType();
		ConstantBuffer::Parameter *cbVar = new ConstantBuffer::Parameter();
		D3D11_SHADER_VARIABLE_DESC &v = cbVar->Variable;
		D3D11_SHADER_TYPE_DESC &t = cbVar->Type;
		DX(var->GetDesc(&v));
		DX(type->GetDesc(&t));
		Parameters[v.Name] = cbVar;
		TRACE("    Parameter: %s%s : %s %s%d %s\n",	v.Name,
														(t.Elements > 0) ? cFormat("[%d]", t.Elements).c_str() : "",
														classNames[t.Class],
														isMatrix[t.Class] ? cFormat("%dx", t.Rows).c_str() : "",
														t.Columns,
														typeNames[t.Type]
														);
	}
	return S_OK;
}

