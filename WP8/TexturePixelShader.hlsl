Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float2 tex : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 diffuseTexture = Texture.Sample(Sampler, input.tex);
	//diffuseTexture.a = 0.9f;
	//return diffuseTexture;
	return diffuseTexture * input.color;
	//return input.color;
}
