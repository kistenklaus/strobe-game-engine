
#define PI 3.14159265358979323846

struct PushConstants {
  uint particlePositionsIndex;
  uint particleVelocityIndex;
  uint particleDensitiesIndex;

  uint particleCount;
  float deltaTime;
  float particleMass;

  float smoothingRadius;

  float2 viewportPos;
  float2 viewportExtent;

};

[[vk::push_constant]]
PushConstants pc;

float2 predict_position(float2 x, float2 v, float dt) {
  return x + v * dt;
}

void add_density(float2 x_i, float2 x_j, float h2, inout float density) {
  const float2 delta = x_i - x_j;
  const float r2 = dot(delta, delta);

  if (r2 < h2) {
    const float q = h2 - r2;
    density += q * q * q;
  }
}

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID) {
  const uint i = threadId.x;
  const uint N = pc.particleCount;

  if (i >= N) {
    return;
  }

  StructuredBuffer<float2> positions = ResourceDescriptorHeap[pc.particlePositionsIndex];
  StructuredBuffer<float2> velocities = ResourceDescriptorHeap[pc.particleVelocityIndex];
  RWStructuredBuffer<float> densities = ResourceDescriptorHeap[pc.particleDensitiesIndex];

  const float dt = pc.deltaTime;
  const float m = pc.particleMass;
  const float h = pc.smoothingRadius;
  const float h2 = h * h;
  const float h4 = h2 * h2;
  const float h8 = h4 * h4;
  const float c_0 = (4.0 * m) / (PI * h8);

  const float2 viewportPos = pc.viewportPos;
  const float2 viewportExtent = pc.viewportExtent;
  const float2 minPos = viewportPos;
  const float2 maxPos = viewportPos + viewportExtent;

  const float2 x_i = predict_position(positions[i], velocities[i], dt);

  const bool nearLeft   = x_i.x - minPos.x < h;
  const bool nearRight  = maxPos.x - x_i.x < h;
  const bool nearTop    = x_i.y - minPos.y < h;
  const bool nearBottom = maxPos.y - x_i.y < h;


  float density = 0.0f;
  for (uint j = 0; j < N; ++j) {
    const float2 x_j = predict_position(positions[j], velocities[j], pc.deltaTime);
    add_density(x_i, x_j, h2, density);
    const float left   = 2.0f * minPos.x - x_j.x;
    const float right  = 2.0f * maxPos.x - x_j.x;
    const float top    = 2.0f * minPos.y - x_j.y;
    const float bottom = 2.0f * maxPos.y - x_j.y;
    if (nearLeft) {
      add_density(x_i, float2(left, x_j.y), h2, density);
    }
    if (nearRight) {
      add_density(x_i, float2(right, x_j.y), h2, density);
    }
    if (nearTop) {
      add_density(x_i, float2(x_j.x, top), h2, density);
    }
    if (nearBottom) {
      add_density(x_i, float2(x_j.x, bottom), h2, density);
    }

    if (nearLeft && nearTop) {
      add_density(x_i, float2(left, top), h2, density);
    }
    if (nearLeft && nearBottom) {
      add_density(x_i, float2(left, bottom), h2, density);
    }
    if (nearRight && nearTop) {
      add_density(x_i, float2(right, top), h2, density);
    }
    if (nearRight && nearBottom) {
      add_density(x_i, float2(right, bottom), h2, density);
    }
  }

  densities[i] = density * c_0;

}
