//////////////////////////////////////////////////////////////////////

Texture2D pic : register(t0);
SamplerState smplr : register(s0);

//////////////////////////////////////////////////////////////////////

cbuffer PixelShaderConstants : register(b0)
{
	float4 mask;
	float4 offset;
	float4 gridColor0;
	float4 gridColor1;
	float2 gridSize;
	float2 gridSize2;
}

//////////////////////////////////////////////////////////////////////

struct PS_INPUT
{
	float4 Pos : SV_Position;
	float2 Tex0 : TEXCOORD0;
	float2 Tex1 : TEXCOORD1;
};

//////////////////////////////////////////////////////////////////////

float4 main(PS_INPUT input) : SV_Target
{
	float y = trunc(fmod(input.Tex1.y, gridSize2.y) / gridSize.y) * gridSize.x;
	float x = trunc(fmod(input.Tex1.x + y, gridSize2.x) / gridSize.x);
	float4 picColour = pic.Sample(smplr, input.Tex0) * mask + offset;
	float4 gridColor = lerp(gridColor0, gridColor1, x);
	return lerp(gridColor, picColour, picColour.a);
}
