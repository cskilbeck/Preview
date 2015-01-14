//////////////////////////////////////////////////////////////////////

cbuffer VertConstants
{
	float4x4 ProjectionMatrix;
}

//////////////////////////////////////////////////////////////////////

struct VS_INPUT
{
	float2 pos : POSITION;
	uint col : COLOR0;
};

//////////////////////////////////////////////////////////////////////

struct PS_INPUT
{
	float4 Pos : SV_Position;
	float4 col : COLOR0;
};

//////////////////////////////////////////////////////////////////////

PS_INPUT main(VS_INPUT input)
{
	float r = (input.col & 0xff) / 255.0f;
	float g = ((input.col >> 8) & 0xff) / 255.0f;
	float b = ((input.col >> 16) & 0xff) / 255.0f;
	float a = ((input.col >> 24) & 0xff) / 255.0f;
	PS_INPUT o;
	o.Pos = mul(float4(input.pos.x, input.pos.y, 0.5, 1.0), ProjectionMatrix);
	o.col = float4(r, g, b, a);
	return o;
}

//////////////////////////////////////////////////////////////////////

