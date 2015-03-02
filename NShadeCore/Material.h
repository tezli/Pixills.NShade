#pragma once

#include "includes.h"
#include "shader.h"

class Material
{
public:
	Material();
	~Material();

public:
	XMFLOAT4					Color;
	shared_ptr<ID3D11Texture2D>	Texture;
	shared_ptr<ID3D11Texture2D>	BumpMap;
	shared_ptr<ID3D11Texture2D>	DisplaceMentMap;
	shared_ptr<vector<Shader>>	Shaders;
	float						SpecularPower;
};

