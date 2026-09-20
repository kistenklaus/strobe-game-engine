
struct PushConstants
{
    float2 viewportOffset;
    float2 viewportExtent;
    float  particleRadius;
};

[[vk::push_constant]]
PushConstants pc;

struct VertexInput
{
    [[vk::location(0)]]
    float2 circlePosition;

    [[vk::location(1)]]
    float2 particlePosition;
};

struct VertexOutput
{
    float4 position : SV_Position;

    [[vk::location(0)]]
    float2 localPosition;
};

[shader("vertex")]
VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.localPosition = input.circlePosition;
    float2 worldPosition =
        input.particlePosition +
        pc.particleRadius * input.circlePosition;
    float2 uv =
        (worldPosition - pc.viewportOffset) / pc.viewportExtent;
    float2 ndc = 2.0 * uv - 1.0;
    output.position = float4(ndc, 0.0, 1.0);
    return output;
}
