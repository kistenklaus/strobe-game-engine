struct PushConstants
{
    float2 scale;
    float2 translate;
    uint textureIndex;
    uint samplerIndex;
};

[[vk::push_constant]]
PushConstants pc;

struct FragmentInput
{
    [[vk::location(0)]]
    float2 uv : TEXCOORD0;

    [[vk::location(1)]]
    float4 color : COLOR0;
};

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    Texture2D<float4> texture =
        ResourceDescriptorHeap[pc.textureIndex];

    SamplerState sampler =
        SamplerDescriptorHeap[pc.samplerIndex];

    return input.color * texture.Sample(sampler, input.uv);
}
