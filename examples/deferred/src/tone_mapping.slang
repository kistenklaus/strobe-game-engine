
struct PushConstants {
  uint2 extent;
  uint inputIndex;
  uint outputIndex;
}

[[vk::push_constant]]
PushConstants pc;


float3 linear_srgb_to_nonlinear_srgb(float3 linear_srgb) {
  float3 result;
  for (int i = 0; i < 3; ++i) {
    const float x = linear_srgb[i];
    result[i] = x <= 0.0031308f
                    ? 12.92f * x
                    : 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
  }
  return result;
}

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
  
  output[pixel] = float4(linear_srgb_to_nonlinear_srgb(input[pixel].rgb), 1.0f);
}
