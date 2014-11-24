float4 main(float2 pos : POSITION, float2 uv: TEXCOORD) : SV_POSITION
{
	return float4(pos.x, pos.y, 0, 1);;
}