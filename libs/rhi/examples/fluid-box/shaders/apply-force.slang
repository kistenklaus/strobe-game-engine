
#define PI 3.14159265358979323846

struct PushConstants {
  uint particlePositionReadIndex;
  uint particlePositionWriteIndex;
  uint particleVelocityReadIndex;
  uint particleVelocityWriteIndex;
  uint particleDensityReadIndex;

  uint particleCount;
  float deltaTime;
  float particleMass;

  float restDensity;
  float pressureStiffness;
  float viscosity;
  float smoothingRadius;

  float linearDamping;
  float restitution;

  float2 externalForce;
  float2 viewportPos;
  float2 viewportExtent;
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
  const uint particleCount = pc.particleCount;

  if (i >= particleCount)
    return;

  StructuredBuffer<Particle> readParticles =
    ResourceDescriptorHeap[pc.particleReadIndex];

  RWStructuredBuffer<Particle> writeParticles =
    ResourceDescriptorHeap[pc.particleWriteIndex];

  const float dt = pc.deltaTime;
  const float mass = pc.particleMass;
  const float2 externalForce = pc.externalForce;

  Particle self = readParticles[i];

  // 1. external acceleration
  float2 acceleration = externalForce / mass;

  const float h = pc.smoothingRadius;
  const float h2 = h*h;
  const float h4 = h2 * h2;
  const float h5 = h4 * h;
  
  // 2. pressure acceleration
  float self_pressure = pc.pressureStiffness * max(self.density - pc.restDensity, 0);
  float self_dentity2 = self.density * self.density;
  for (uint j = 0; j < particleCount; ++j) {
    if (i == j) {
      continue;
    }

    Particle other = readParticles[j];

    float2 delta = self.position - other.position;
    float r2 = dot(delta, delta);
    if (r2 < h2 && r2 >  1e-12f * h2) {
      float r = sqrt(r2);
      float2 direction = delta / r;
      float gradientMag = -30.0f / (PI * h5) * pow(h-r, 2);
      float2 gradient = gradientMag * direction;

      float other_pressure = pc.pressureStiffness * max(other.density - pc.restDensity, 0);
      float other_density2 = other.density * other.density;

      // pressure
      acceleration -= mass 
        * (self_pressure / self_dentity2 + other_pressure  / other_density2)
        * gradient;
      // viscosity
      float laplacian =
        40.0f / (PI * h5) * (h - r);

      acceleration +=
        pc.viscosity * mass *
        (other.velocity - self.velocity) *
        laplacian /
        (self.density * other.density);
    }
  }



  self.velocity += acceleration * dt;
  self.velocity *= exp(-pc.linearDamping * dt);
  self.position += self.velocity * dt;

  const float2 minPos = pc.viewportPos;
  const float2 maxPos = pc.viewportPos + pc.viewportExtent;
  const float restitution = saturate(pc.restitution);

  if (self.position.x < minPos.x) {
    self.position.x = minPos.x;

    if (self.velocity.x < 0.0)
      self.velocity.x *= -restitution;
  }

  if (self.position.x > maxPos.x) {
    self.position.x = maxPos.x;

    if (self.velocity.x > 0.0)
      self.velocity.x *= -restitution;
  }

  if (self.position.y < minPos.y) {
    self.position.y = minPos.y;

    if (self.velocity.y < 0.0)
      self.velocity.y *= -restitution;
  }

  if (self.position.y > maxPos.y) {
    self.position.y = maxPos.y;

    if (self.velocity.y > 0.0)
      self.velocity.y *= -restitution;
  }



  writeParticles[i] = self;
}
