//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

// define platform

#include "MaterialImpl.h"

struct MaterialImpl
{
	DXPtr<ID3D11VertexShader> mVertexShader;

	MaterialImpl()
	{

	}

	~MaterialImpl()
	{

	}

	void Load(char const *filename)
	{
	}

	void Unload()
	{
	}

	void Activate()
	{
	}

};

struct Material : MaterialImpl
{
	Material()
	{
	}

	Material(char const *filename)
	{
		Load(filename);
	}

	virtual ~Material()
	{
		Unload();
	}
};
