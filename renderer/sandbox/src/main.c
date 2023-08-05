#include "frame_guard.h"
#include "mat.h"
#include "mesh.h"
#include "trigon.h"
#include "render_backend.h"
#include "scene.h"
#include "render_context.h"
#include "render_loop.h"
#include "shared_memory.h"
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
  create_glfw_window(&window, 800, 600, "FLOATING");

  render_backend_create_info backendCreateInfo;

  render_context_create_info contextCreateInfo;
  contextCreateInfo.window = &window;
  contextCreateInfo.swapBufferCount = 0;
  contextCreateInfo.backend = backendCreateInfo;
  render_context_t context;

  render_context_init(&context, &contextCreateInfo);
  renderer_start(&context);

  unsigned int scene;
  scene_create_info sceneCreateInfo = {};
  sceneCreateInfo.layer = 0;
  sceneCreateInfo.renderer = TRIGON_SCENE_RENDERER;
  renderer_create_scene(&context, &scene, &sceneCreateInfo);

  unsigned int memory_buffer;
  float data[] = {0, 0, 0};
  shared_memory meshVertexData;
  shared_memory_create(&meshVertexData, sizeof(float) * (3 + 2 + 3));


  unsigned int mesh;
  mesh_create_info meshCreateInfo = {};
  meshCreateInfo.positions.data = meshVertexData;
  meshCreateInfo.positions.size = 3;
  meshCreateInfo.positions.usage = STROBE_MESH_ATTRIBUTE_USAGE_STATIC;
  meshCreateInfo.positions.offset = 0;
  meshCreateInfo.positions.stride = 3;
  /* meshCreateInfo.textureCoords = MESH_NO_ATTIBUTE_CREATE_INFO; */
  /* meshCreateInfo.normals = MESH_NO_ATTIBUTE_CREATE_INFO; */
  /* meshCreateInfo.tangents = MESH_NO_ATTIBUTE_CREATE_INFO; */
  /* meshCreateInfo.bitangents = MESH_NO_ATTIBUTE_CREATE_INFO; */
  /* meshCreateInfo.colors = MESH_NO_ATTIBUTE_CREATE_INFO; */
  meshCreateInfo.vertexCount = 1;
  
  renderer_create_mesh(&context, &mesh, &meshCreateInfo);

  mat4f transform;
  mat4f_identity(&transform);


  while (renderer_is_running(&context)) {
    renderer_begin_frame(&context);

    //renderer_draw(&context,mesh, material, &transform);
    //renderer_draw(&context, scene, mesh, material, &transform);

    renderer_end_frame(&context);
  }

  renderer_stop(&context);

  render_context_destroy(&context);
}
