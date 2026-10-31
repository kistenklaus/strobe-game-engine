
struct PushConstants {
  uint2 extent;
  uint inputIndex;
  uint outputIndex;
}

[[vk::push_constant]]
PushConstants pc;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 threadId : SV_DispatchThreadID) {
  uint2 pixel = threadId.xy;
  if (any(pixel >= pc.extent)) {
    return;
  }

  RWTexture2D<float4> input =
      ResourceDescriptorHeap[pc.inputIndex];
  RWTexture2D<float4> output =
      ResourceDescriptorHeap[pc.outputIndex];

  output[pixel] = float4(0.5f, 0.5f, 0.5f, 1.0f);
}
