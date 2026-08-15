
[shader("vertex")]
float4 vertexMain(uint vertexID : SV_VertexID) : SV_Position
{
    float2 positions[3] = {
        float2(-1.0, -1.0),
        float2( 3.0, -1.0),
        float2(-1.0,  3.0)
    };

    return float4(positions[vertexID], 0.0, 1.0);
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}
