
struct PushConstants {
  uint particleCount;
  float deltaTime;
  float mass;
  float linearDamping;
  float restitution;
  float2 externalForce;
  float2 viewportPos;
  float2 viewportExtent;
  uint particleIndex;
};

[[vk::push_constant]]
PushConstants pc;

struct Particle {
  float2 position;
  float2 velocity;
};

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
  uint i = threadId.x;

  if (i >= pc.particleCount)
    return;

  RWStructuredBuffer<Particle> particles =
    ResourceDescriptorHeap[pc.particleIndex];

  const float dt = pc.deltaTime;
  const float mass = pc.mass;
  const float2 externalForce = pc.externalForce;

  Particle p = particles[i];

  float2 acceleration = externalForce / mass;

  p.velocity += acceleration * dt;
  p.velocity *= exp(-pc.linearDamping * dt);
  p.position += p.velocity * dt;

  const float2 minPos = pc.viewportPos;
  const float2 maxPos = pc.viewportPos + pc.viewportExtent;
  const float restitution = saturate(pc.restitution);

  if (p.position.x < minPos.x) {
    p.position.x = minPos.x;

    if (p.velocity.x < 0.0)
      p.velocity.x *= -restitution;
  }

  if (p.position.x > maxPos.x) {
    p.position.x = maxPos.x;

    if (p.velocity.x > 0.0)
      p.velocity.x *= -restitution;
  }

  if (p.position.y < minPos.y) {
    p.position.y = minPos.y;

    if (p.velocity.y < 0.0)
      p.velocity.y *= -restitution;
  }

  if (p.position.y > maxPos.y) {
    p.position.y = maxPos.y;

    if (p.velocity.y > 0.0)
      p.velocity.y *= -restitution;
  }

  particles[i] = p;
}
