#pragma pack_matrix (row_major)

cbuffer b0 : register(b0) { matrix MatrixTransform; }
cbuffer b1 : register(b1) { float4 global_color; }
cbuffer b2 : register(b2) { bool use_global_color; };

struct VertexShaderInput
{
	float3 pos : POSITION;
	float4 color : COLOR0;
    float2 tex : TEXCOORD0;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
    float2 tex : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	output.pos = mul(float4(input.pos, 1.0f), MatrixTransform);
	output.color = (use_global_color ? global_color : input.color);
	output.tex = input.tex;
	return output;
}
