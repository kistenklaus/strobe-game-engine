#define PI 3.14159265358979323846

struct PushConstants {
  uint particleReadIndex;
  uint particleWriteIndex;
  uint particleCount;
  float particleMass;
  float smoothingRadius;
};

[[vk::push_constant]]
PushConstants pc;

struct Particle {
  float2 position;
  float2 velocity;
  float density;
  float _padding;
};

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
  uint i = threadId.x;
  uint particleCount = pc.particleCount;

  if (i >= particleCount)
    return;

  StructuredBuffer<Particle> readParticles =
    ResourceDescriptorHeap[pc.particleReadIndex];
  RWStructuredBuffer<Particle> writeParticles =
    ResourceDescriptorHeap[pc.particleWriteIndex];

  Particle self = readParticles[i];

  float h2 = pc.smoothingRadius * pc.smoothingRadius;
  float h4 = h2 * h2;
  float h8 = h4 * h4;
  float factor = 4.0f / (PI * h8);

  float density = 0.0f;

  for (uint j = 0; j < particleCount; ++j) {
    Particle other = readParticles[j];

    float2 delta = self.position - other.position;
    float r2 = dot(delta, delta);

    if (r2 < h2) {
      float x = h2 - r2;
      density += x * x * x;
    }
  }

  self.density = pc.particleMass * factor * density;
  writeParticles[i] = self;
}
