#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Particle {
  vec3 position;
  float density; // 4
  vec3 velocity;
  float pressure; // 8
  vec3 acceleration;
  float hash; // 12
  vec3 force;
  float _pad0; // 16
  vec3 normal;
  float _pad1; // 20
};

layout(std430, binding = 0) buffer ParticleBlock { Particle particles[]; };

const float PI = 3.1415927410125732421875f;

uniform int particles_size;
uniform int num_cells;
uniform float time;
uniform float h;
uniform float MASS;
uniform float GAS_CONST;
uniform float REST_DENS;

float poly6(float r) {
  return 315.0f * pow(h * h - r * r, 3.0f) / (64.0f * PI * pow(h, 9.0f));
}

int hash(vec3 position) {
  vec3 p_hat = floor(position / h);
  return ((int(p_hat.x) * 73856093) ^ (int(p_hat.y) * 19349663) ^
          (int(p_hat.z) * 83492791)) %
         num_cells;
}

void main() {
  uint i = gl_WorkGroupID.x;
  float dens = 0;
  Particle p = particles[i];

  for (uint j = 0; j < particles_size; j++) {
    float r = distance(p.position, particles[j].position);
    if (r < h) {
      dens += MASS * poly6(r);
    }
  }

  particles[i].density = dens;
  particles[i].pressure = GAS_CONST * (dens - REST_DENS);
}