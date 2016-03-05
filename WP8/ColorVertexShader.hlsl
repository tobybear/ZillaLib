#pragma pack_matrix (row_major)

cbuffer b0 : register(b0) { matrix MatrixTransform; }
cbuffer b1 : register(b1) { float4 global_color; }
cbuffer b2 : register(b2) { bool use_global_color; };

struct VertexShaderInput
{
	float3 pos : POSITION;
	float4 color : COLOR0;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	//float4 pos = float4(input.pos, 1.0f);
	//pos.z = 0.0f;

	// Transform the vertex position into projected space.
	//pos = mul(pos, model);
	//pos = mul(pos, view);
	//pos = mul(pos, projection);
	//output.pos = pos;
	output.pos = mul(float4(input.pos, 1.0f), MatrixTransform);

	// Pass through the color without modification.
	//output.color = input.color;
	//output.color.r = 1;
	//output.color.g = 0;
	//output.color.b = 0;
	//output.color.a = 0.8;
	output.color = (use_global_color ? global_color : input.color);
	//output.color.a = 0.8;

	return output;
}
