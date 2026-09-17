struct PushConstants
{
    float2 scale;
    float2 translate;
    uint textureIndex;
    uint samplerIndex;
};

[[vk::push_constant]]
PushConstants pc;

struct VertexInput
{
    [[vk::location(0)]]
    float2 position : POSITION;

    [[vk::location(1)]]
    float2 uv : TEXCOORD0;

    [[vk::location(2)]]
    float4 color : COLOR0;
};

struct VertexOutput
{
    float4 position : SV_Position;

    [[vk::location(0)]]
    float2 uv : TEXCOORD0;

    [[vk::location(1)]]
    float4 color : COLOR0;
};

[shader("vertex")]
VertexOutput main(VertexInput input)
{
    VertexOutput output;

    output.position = float4(
        input.position * pc.scale + pc.translate,
        0.0,
        1.0);

    output.uv = input.uv;
    output.color = input.color;

    return output;
}
