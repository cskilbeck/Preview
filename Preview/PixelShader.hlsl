Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
    float2 Pos : SV_Position;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_Position;
	float2 Tex : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_Target
{
	return txDiffuse.Sample(samLinear, input.Tex);
}
