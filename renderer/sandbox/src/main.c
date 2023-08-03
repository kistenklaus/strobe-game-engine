#include "frame_guard.h"
#include "mat.h"
#include "material.h"
#include "mesh.h"
#include "phashtable.h"
#include "render_context.h"
#include "render_loop.h"
#include "shared_memory.h"
#include "thread.h"
#include "trigon.h"
#include <renderer.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "renderer_submission.h"

#include "glfw_window.h"

int main(int argc, char **argv) {

  mat4f a;
  mat4f b;
  float x[2];
  mat4f_mul(&a, &a, &b);

  window_t window;
  create_glfw_window(&window, 800, 600, "Hello, float!");

  render_backend_create_info backendCreateInfo;

  render_context_create_info contextCreateInfo;
  contextCreateInfo.window = &window;
  contextCreateInfo.swapBufferCount = 0;
  contextCreateInfo.backend = TRIGON_BACKEND;
  render_context_t context;
  render_context_init(&context, &contextCreateInfo);

  unsigned int memory_buffer;
  float data[] = {0, 0, 0};
  shared_memory meshVertexData;
  shared_memory_create(&meshVertexData, sizeof(float) * (3 + 2 + 3));

  unsigned int mesh;
  mesh_create_info meshCreateInfo;
  meshCreateInfo.textureCoords = MESH_NO_ATTIBUTE_CREATE_INFO;
  meshCreateInfo.normals = MESH_NO_ATTIBUTE_CREATE_INFO;
  meshCreateInfo.tangents = MESH_NO_ATTIBUTE_CREATE_INFO;
  meshCreateInfo.bitangents = MESH_NO_ATTIBUTE_CREATE_INFO;
  renderer_create_mesh(&context, &mesh, &meshCreateInfo);
  mesh_mapped_attribute positionAttribute;
  renderer_map_mesh_attribute(&context, mesh, MESH_ATTRIBUTE_POSITION,
                              &positionAttribute);
  float positionData[] = {-1.0f, -1.0f, 0.0f, -1.0f, 1.0f,  0.0f,
                          1.0f,  1.0f,  0.0f, 1.0f,  -1.0f, 0.0f};
  renderer_set_mapped_mesh_attribute_range(&positionAttribute, 0, positionData,
                                           4 * 3);
  renderer_unmap_mesh_attribute(&context, &positionAttribute);

  unsigned int material;
  material_create_info materialCreateInfo;
  float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  materialCreateInfo.initalPropertyValues = &color;
  renderer_get_material_description_by_name(
      &context, &materialCreateInfo.description, "just-color");
  renderer_create_material(&context, &material, &materialCreateInfo);

  mat4f transform;
  mat4f_identity(&transform);

  while (!window.shouldClose) {
    renderer_begin_frame(&context);

    renderer_draw(&context,mesh, material, &transform);

    renderer_end_frame(&context);
  }

  renderer_destroy_mesh(&context, mesh);
  renderer_destroy_material(&context, material);

  render_context_destroy(&context);
}
