struct PS_INPUT
{
	float4 Pos : SV_Position;
	float2 Tex : TEXCOORD0;
};

PS_INPUT main(float2 pos : POSITION, float2 uv : TEXCOORD)
{
	PS_INPUT o;
	o.Pos = float4(pos.x, pos.y, 0, 1);
	o.Tex = uv;
	return o;
}