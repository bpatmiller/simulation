#ifndef PARTICLECONTAINER_H
#define PARTICLECONTAINER_H

#include "Particle.h"
#include "VAO.h"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>

struct ParticleContainer {
  // render objects
  VertexArr VAO;
  glm::mat4 model_matrix;
  std::vector<glm::vec3> vertices;
  std::vector<glm::uvec3> face_indices;
  // instance vector
  std::vector<glm::vec4> positions;
  // particles and particle - hash map
  std::vector<Particle> particles;
  std::unordered_multimap<int, Particle *> block_hashmap;
  // simulation parameters
  float timestep = 0.005f;
  float particle_radius = 0.05;
  float h = particle_radius * 4;
  float GRID_SIZE = h;
  int NUM_CELLS;
  // fluid coefficients
  float MASS = 28.0;
  float REST_DENSITY = 59.0;
  float GAS_CONST = 0.0000001;
  float VISC = 0.8;
  float SURF = 8.0;
  // container/tuning parameters
  glm::vec3 container_min;
  glm::vec3 container_max;
  float damping = -0.75;
  // math coefficients
  const float PI = 3.14159265358979323846264338327950288;
  const float POLY6_COEF = 315.0f / (64 * PI * glm::pow(h, 9));
  const float POLY6_GRAD_COEF = -945.0f / (32 * PI * glm::pow(h, 9));
  const float SPIKY_COEF = 15.0f / (PI * glm::pow(h, 6.0f));
  const float spiky_grad_coef = 45.0f / (PI * glm::pow(h, 6));
  const float laplacian_visc_coef = 45.0f / (PI * glm::pow(h, 6));
  const float C_coef = (32.0f / (PI * glm::pow(h, 9)));
  // toggles
  bool fountain = true;

  ParticleContainer(glm::vec3 min, glm::vec3 max, glm::vec3 c_min,
                    glm::vec3 c_max) {
    // set bounds
    container_min = c_min;
    container_max = c_max;

    // load mesh
    create_sphere(particle_radius);

    // init scene
    init_sph(min, max);

    NUM_CELLS = (max.x - min.x) * (max.y - min.y) * (max.z - min.z) /
                glm::pow(GRID_SIZE, 3.0f);

    // bind vao
    VAO.vb.bindVertices(vertices);
    VAO.ib.bindVertices(positions);
    VAO.setLayout({3}, false);
    VAO.setLayout({4}, true);
    // define cube scale
    model_matrix = glm::scale(glm::vec3(1));
  }
  void draw();
  void update_instances();
  void create_sphere(float Radius);
  void create_sprite(float Radius);

  void init_sph(glm::vec3 min, glm::vec3 max) {
    // generate initial positions
    float dens = 2.0f;
    for (float x = min.x; x <= max.x; x += h / dens) {
      for (float y = min.y; y <= max.y; y += h / dens) {
        for (float z = min.z; z <= max.z; z += h / dens) {
          float jitter =
              0.01f * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
          Particle p = Particle(glm::vec3(x, y, z + jitter));
          p.color = (x - min.x) / (max.x - min.x);
          particles.emplace_back(p);
          positions.emplace_back(glm::vec4(p.position, p.density));
        }
      }
    }
  };

  // fluid sim helper stems
  int hash(glm::vec3);
  float poly6(float r);
  glm::vec3 poly6_grad(glm::vec3 r);
  float poly6_laplac(glm::vec3 r);
  glm::vec3 spiky_grad(glm::vec3 r);
  float laplacian_visc(glm::vec3 r);
  float spiky(float r);

  float C(float r);
  // fluid simulation steps
  void compute_neighbors_density();
  void compute_pressure();
  void compute_forces();
  void integrate();
  void step_physics(int n);
};

#endif