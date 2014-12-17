//////////////////////////////////////////////////////////////////////

Texture2D picture;

SamplerState samplerState;

//////////////////////////////////////////////////////////////////////

const float2 bill;

cbuffer ColourModifiers
{
	float4 ChannelMask = float4(1, 1, 1, 1);
	float4 ColorOffset = float4(0, 0, 0, 0);
}

cbuffer GridStuff
{
	float4 GridColor0 = float4(0.8,0.8,0.8,1);
	float4 GridColor1 = float4(0.6,0.6,0.6,1);
	float2 GridSize = float2(16, 16);
	float2 GridSize2 = float2(32, 32);
}

const float4 off;

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
	float4 picColour = picture.Sample(samplerState, input.Tex0) * ChannelMask + ColorOffset + off + float4(bill.x, bill.y, 0, 0);
	float y = trunc(fmod(input.Tex1.y, GridSize2.y) / GridSize.y) * GridSize.x;
	float x = trunc(fmod(input.Tex1.x + y, GridSize2.x) / GridSize.x);
	float4 gridColor = lerp(GridColor0, GridColor1, x);
	return lerp(gridColor, picColour, picColour.a);
}
