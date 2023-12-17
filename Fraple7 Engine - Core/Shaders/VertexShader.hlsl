
struct Output
{
	float4 Color : Color;
	float4 Position : SV_Position;
};
Output main(float3 pos : Position, float3 color : Color)
{
	Output vertexOut;
	vertexOut.Position = float4(pos, 1.0f);
	vertexOut.Color = float4(color, 1.0f);

	return vertexOut;
}