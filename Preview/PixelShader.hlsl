//////////////////////////////////////////////////////////////////////

Texture2D pic : register(t0);
SamplerState smplr : register(s0);

//////////////////////////////////////////////////////////////////////

cbuffer PixelShaderConstants : register(b0)
{
	float4 ChannelMask;
	float4 ColorOffset;
	float4 GridColor0;
	float4 GridColor1;
	float2 GridSize;
	float2 GridSize2;
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
	float4 picColour = pic.Sample(smplr, input.Tex0) * ChannelMask + ColorOffset;
	float y = trunc(fmod(input.Tex1.y, GridSize2.y) / GridSize.y) * GridSize.x;
	float x = trunc(fmod(input.Tex1.x + y, GridSize2.x) / GridSize.x);
	float4 gridColor = lerp(GridColor0, GridColor1, x);
	return lerp(gridColor, picColour, picColour.a);
}
