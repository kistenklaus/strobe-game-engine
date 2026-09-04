struct VertexInput
{
    float2 position : POSITION;
};

struct VertexOutput
{
    float4 position : SV_Position;
};

struct ColorUniform
{
    // Using float4 avoids float3 host-layout ambiguity.
    float4 color;
};

// Byte offset 0 in push data.
[vk::push_constant]
uint colorDescriptorIndex;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position, 0.0, 1.0);
    return output;
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    ConstantBuffer<ColorUniform> colorBuffer =
        ResourceDescriptorHeap[colorDescriptorIndex];

    return colorBuffer.color;
}
