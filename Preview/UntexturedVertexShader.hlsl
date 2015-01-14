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
	uint r = input.col & 0xff;
	uint g = (input.col >> 8) & 0xff;
	uint b = (input.col >> 16) & 0xff;
	uint a = (input.col >> 24) & 0xff;
	float fr = r / 255.0f;
	float fg = g / 255.0f;
	float fb = b / 255.0f;
	float fa = a / 255.0f;
	PS_INPUT o;
	o.Pos = mul(float4(input.pos.x, input.pos.y, 0.5, 1.0), ProjectionMatrix);
	o.col = float4(fr, fg, fb, fa);
	return o;
}

//////////////////////////////////////////////////////////////////////

