struct PushConstants
{
    uint textureIndex;
    uint samplerIndex;
};

[[vk::push_constant]]
PushConstants pc;

struct FragmentInput
{
    [[vk::location(0)]]
    float2 uv : TEXCOORD0;
};

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    Texture2D<float> densityMap =
        ResourceDescriptorHeap[pc.textureIndex];

    SamplerState sampler =
        SamplerDescriptorHeap[pc.samplerIndex];

    float density = densityMap.Sample(sampler, input.uv);

    // Exposure-map the unbounded density to [0, 1].
    float t = 1.0 - exp(-1.5 * density);

    // Dark blue background -> blue -> bright cyan.
    float3 background = float3(0.008, 0.012, 0.020);
    float3 low        = float3(0.020, 0.150, 0.350);
    float3 high       = float3(0.350, 0.850, 1.000);

    float3 fluid = lerp(low, high, smoothstep(0.0, 1.0, t));
    float3 color = lerp(background, fluid, t);

    return float4(color, 1.0);
}
