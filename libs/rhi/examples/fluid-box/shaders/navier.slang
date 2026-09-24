
#define PI 3.14159265358979323846

struct PushConstants {
  uint particlePositionsIndex;
  uint particleVelocitiesIndex;
  uint particleDensitiesIndex;
  uint particleForcesIndex;

  uint particleCount;
  float deltaTime;

  float restDensity;
  float pressureStiffness;
  float viscosity;
  float smoothingRadius;

  float2 viewportPos;
  float2 viewportExtent;
};

[[vk::push_constant]]
PushConstants pc;

float2 predict_position(float2 x, float2 v, float dt) {
  return x + v * dt;
}

float pressure(float rho, float rho_0, float k) {
  return k * max(rho - rho_0,0);
}

void add_force(
    float2 x_i, float2 v_i, float rho_i, float alpha_i,
    float2 x_j, float2 v_j, float rho_j, float alpha_j,
    float h, float h2, float c_0, float c_1, float mu,
    float rho_0,
    inout float2 force) {
  const float2 delta = x_i - x_j;
  const float r2 = dot(delta, delta);

  if (r2 >= h2 || r2 <= 1e-12f * h2) {
    return;
  }

  const float r = sqrt(r2);
  // contact force (properly integrate me!)
  float mass = 625.0f;
  const float contactRadius =
    0.5f * sqrt(mass / rho_0); // ~8.8 px
  if (r < contactRadius) {
    const float strength = 5000.0f * (1.0f - r / contactRadius);
    force += (strength / mass) * delta / r;
  }


  const float hr = h - r;

  const float2 grad = (c_0 * hr * hr) * delta / r;
  force -= (alpha_i + alpha_j) * grad;

  const float laplacian = c_1 * hr;
  force += mu * (v_j - v_i) * laplacian / (rho_i * rho_j);
}

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID) {

  const uint i = threadId.x;
  const uint N = pc.particleCount;

  if (i >= N)
    return;

  StructuredBuffer<float2> positions =
    ResourceDescriptorHeap[pc.particlePositionsIndex];
  StructuredBuffer<float2> velocities =
    ResourceDescriptorHeap[pc.particleVelocitiesIndex];
  StructuredBuffer<float> densities =
    ResourceDescriptorHeap[pc.particleDensitiesIndex];
  RWStructuredBuffer<float2> forces =
    ResourceDescriptorHeap[pc.particleForcesIndex];

  const float dt = pc.deltaTime; 
  const float2 viewportPos = pc.viewportPos;
  const float2 viewportExtent = pc.viewportExtent;
  const float rho_0 = pc.restDensity;
  const float k = pc.pressureStiffness;
  const float mu = pc.viscosity;
  const float h = pc.smoothingRadius;
  const float h2 = h * h;
  const float h4 = h2 * h2;
  const float h5 = h4 * h;

  const float2 v_i = velocities[i];
  const float2 x_i = predict_position(positions[i], v_i, dt);
  const float rho_i = densities[i];
  const float rho2_i = rho_i * rho_i;
  const float p_i = pressure(rho_i, rho_0, k);

  const float alpha_i = p_i / rho2_i;

  const float c_0 = -30.0f / (PI * h5);
  const float c_1 = 40.0f / (PI * h5);

  const float2 minPos = viewportPos;
  const float2 maxPos = viewportPos + viewportExtent;
  
  const bool nearLeft   = x_i.x - minPos.x < h;
  const bool nearRight  = maxPos.x - x_i.x < h;
  const bool nearTop    = x_i.y - minPos.y < h;
  const bool nearBottom = maxPos.y - x_i.y < h;

  float2 force = float2(0,0);
  for (uint j = 0; j < N; ++j) {
    const float2 v_j = velocities[j];
    const float2 x_j = predict_position(positions[j], v_j, dt);
    const float rho_j = densities[j];
    const float alpha_j =
      pressure(rho_j, rho_0, k) / (rho_j * rho_j);

    if (j != i) {
      add_force(x_i, v_i, rho_i, alpha_i,
        x_j, v_j, rho_j, alpha_j,
        h, h2, c_0, c_1, mu, rho_0, force);
    }

    const float left = 2.0f * minPos.x - x_j.x;
    const float right = 2.0f * maxPos.x - x_j.x;
    const float top = 2.0f * minPos.y - x_j.y;
    const float bottom = 2.0f * maxPos.y - x_j.y;

    if (nearLeft) {
      add_force(x_i, v_i, rho_i, alpha_i,
        float2(left, x_j.y), float2(-v_j.x, v_j.y), rho_j, alpha_j,
        h, h2, c_0, c_1, mu, rho_0, force);
    }
    if (nearRight) {
      add_force(x_i, v_i, rho_i, alpha_i,
        float2(right, x_j.y), float2(-v_j.x, v_j.y), rho_j, alpha_j,
        h, h2, c_0, c_1, mu, rho_0, force);
    }
    if (nearTop) {
      add_force(x_i, v_i, rho_i, alpha_i,
        float2(x_j.x, top), float2(v_j.x, -v_j.y), rho_j, alpha_j,
        h, h2, c_0, c_1, mu, rho_0, force);
    }
    if (nearBottom) {
      add_force(x_i, v_i, rho_i, alpha_i,
        float2(x_j.x, bottom), float2(v_j.x, -v_j.y), rho_j, alpha_j,
        h, h2, c_0, c_1, mu, rho_0, force);
    }

    if (nearLeft && nearTop) {
      add_force(x_i, v_i, rho_i, alpha_i,
        float2(left, top), -v_j, rho_j, alpha_j,
        h, h2, c_0, c_1, mu, rho_0, force);
    }
    if (nearLeft && nearBottom) {
      add_force(x_i, v_i, rho_i, alpha_i,
        float2(left, bottom), -v_j, rho_j, alpha_j,
        h, h2, c_0, c_1, mu, rho_0, force);
    }
    if (nearRight && nearTop) {
      add_force(x_i, v_i, rho_i, alpha_i,
        float2(right, top), -v_j, rho_j, alpha_j,
        h, h2, c_0, c_1, mu, rho_0, force);
    }
    if (nearRight && nearBottom) {
      add_force(x_i, v_i, rho_i, alpha_i,
        float2(right, bottom), -v_j, rho_j, alpha_j,
        h, h2, c_0, c_1, mu, rho_0, force);
    }
  }

  forces[i] = force;
}
