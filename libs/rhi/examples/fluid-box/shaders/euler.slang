
#define PI 3.14159265358979323846

struct PushConstants {
  uint particlePositionIndex;
  uint particleVelocityIndex;
  uint particleForceIndex;

  uint particleCount;
  float deltaTime;
  float particleMass;

  float linearDamping;
  float restitution;

  float2 externalForce;
  float2 viewportPos;
  float2 viewportExtent;
};

[[vk::push_constant]]
PushConstants pc;

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
  const uint i = threadId.x;
  const uint particleCount = pc.particleCount;

  if (i >= particleCount) {
    return;
  }
  
  RWStructuredBuffer<float2> particlePositions = 
    ResourceDescriptorHeap[pc.particlePositionIndex];
  RWStructuredBuffer<float2> particleVelocities = 
    ResourceDescriptorHeap[pc.particleVelocityIndex];
  StructuredBuffer<float2> particleForces = 
    ResourceDescriptorHeap[pc.particleForceIndex];

  const float dt = pc.deltaTime;
  const float mass = pc.particleMass;
  const float2 viewportPos = pc.viewportPos;
  const float2 viewportExtent = pc.viewportExtent;
  const float c = pc.linearDamping;
  const float epsilon = saturate(pc.restitution);
  const float2 externalForce = pc.externalForce;

  float2 position = particlePositions[i];
  float2 velocity = particleVelocities[i];

  const float2 force = particleForces[i] + externalForce;
  const float2 acceleration = force * mass;
  
  velocity += acceleration * dt;
  velocity *= exp(-c * dt);
  position += velocity * dt;

  const float2 wallInset = float2(9,9);
  const float2 minPos = viewportPos + wallInset;
  const float2 maxPos = viewportPos + viewportExtent - wallInset;

  if (position.x < minPos.x) {
    position.x = minPos.x;
    if (velocity.x < 0.0)  {
      velocity.x *= -epsilon;
    }
  }
  
  if (position.x > maxPos.x) {
    position.x = maxPos.x;
    if (velocity.x > 0.0) {
      velocity.x *= -epsilon;
    }
  }
  
  if (position.y < minPos.y) {
    position.y = minPos.y;
    if (velocity.y < 0.0) {
      velocity.y *= -epsilon;
    }
  }
  
  if (position.y > maxPos.y) {
    position.y = maxPos.y; 
    if (velocity.y > 0.0) {
      velocity.y *= -epsilon;
    }
  }

  particlePositions[i] = position;
  particleVelocities[i] = velocity;
}
