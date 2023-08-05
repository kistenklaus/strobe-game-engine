#include "render_thread.h"
#include "frame_barrier.h"
#include "frame_guard.h"
#include "log.h"
#include "material_manager.h"
#include "mesh_manager.h"
#include "profiler.h"
#include "scene_manager.h"
#include "threading.h"
#include "window.h"
#include <unistd.h>

#ifdef STROBE_RENDERER_ENABLE_PROFILING
static void PROFILER_BEGIN_FRAME(render_context_t *context) {
  frame_profiler_begin_frame(&context->frameProfiler);
}
static void PROFILER_END_FRAME(render_context_t *context) {
  frame_profiler_end_frame(&context->frameProfiler);
}
static void PROFILER_PUSH_BLOCK(render_context_t *context, char *blockName) {
  frame_profiler_push_block(&context->frameProfiler, blockName);
}
static void PROFILER_POP_BLOCK(render_context_t *context) {
  frame_profiler_pop_block(&context->frameProfiler);
}
#else
static void PROFILER_BEGIN_FRAME(render_context_t *context) {}
static void PROFILER_END_FRAME(render_context_t *context) {}
static void PROFILER_PUSH_BLOCK(render_context_t *context, char *blockName) {}
static void PROFILER_POP_BLOCK(render_context_t *context) {}
#endif

static void *render_thread_entry(void *context_arg) {
  render_context_t *context = context_arg;

  logInfo("[Renderer::Setup] Begin\n");
  // Initalize window.
  logInfo("[Renderer::Setup] Initalize window\n");
  window_t *window = context->window;
  window->init(window);
  logDebug("[Renderer::Setup] WindowInfo: \n"
           "      Window-Dimensions : %u x %u\n"
           "      Window-Title      : %s\n",
           window->title);

  unsigned int frameCount = 3;
  /*
  // Initalize backend
  render_backend *backend = &context->backend;
  // backend->init(&frameCount, frameCount); //force frameCount if
  // swapBufferCount != 0.
  if (!frameCount)
    frameCount = context->frameGuard.frameCount;
  logDebug("[Renderer] Successfully initalized render backend\n");
  */

#ifdef STROBE_RENDERER_ENABLE_PROFILING
  logInfo("[Renderer::Setup] Initalize profiler\n");
  frame_profiler_create_info frameProfilerCreateInfo;
  frameProfilerCreateInfo.historySize = 128;
  frame_profiler_init(&context->frameProfiler, &frameProfilerCreateInfo);
#endif

  // Initalize mesh manager
  logInfo("[Renderer::Setup] Initalize mesh manager\n");
  mesh_manager_create_info meshManagerCreateInfo;
  meshManagerCreateInfo.frameCount = frameCount;
  renderer_mesh_manager_init(&context->meshManager, &meshManagerCreateInfo);

  // Initalize material manager
  logInfo("[Renderer::Setup] Initalize material manager\n");
  material_manager_create_info materialManagerCreateInfo;
  materialManagerCreateInfo.frameCount = frameCount;
  renderer_material_manager_init(&context->materialManager,
                                 &materialManagerCreateInfo);

  logInfo("[Renderer::Setup] Initalize scene manager\n");
  scene_manager_create_info sceneManagerCreateInfo;
  sceneManagerCreateInfo.frameCount = frameCount;
#ifdef STROBE_RENDERER_ENABLE_PROFILING
  sceneManagerCreateInfo.frameProfiler = &context->frameProfiler;
#endif
  renderer_scene_manager_init(&context->sceneManager, &sceneManagerCreateInfo);

  // Initalize frame guard.
  logInfo("[Renderer::Setup] Initalize frame guard\n");

  frame_guard_init(&context->frameGuard, frameCount);

  context->running = 1;
  // Signal initalize complete.
  frame_barrier_complete_wait(&context->frameBarrier);

  logInfo("[Renderer::Setup] End\n");

#ifdef STROBE_RENDERER_ENABLE_PROFILING
#ifdef STROBE_LOG_INFOS
  unsigned long long frametimeLogAcc = 7ull * 1000000000ull;
#endif
#endif

  logInfo("[Renderer::Loop] Begin\n");
  while (!(context->shouldClose || window->shouldClose)) {
    PROFILER_BEGIN_FRAME(context);

    PROFILER_PUSH_BLOCK(context, "acquire-frame");
    frame_guard_acquire_complete_frame(&context->frameGuard,
                                       &context->renderFrame);
    PROFILER_POP_BLOCK(context);

    PROFILER_PUSH_BLOCK(context, "window-poll-events");
    window->poll_events(window);
    PROFILER_POP_BLOCK(context);

    PROFILER_PUSH_BLOCK(context, "mesh-manager-prepare");
    //renderer_mesh_manager_prepare_frame(&context->meshManager, context->renderFrame);
    PROFILER_POP_BLOCK(context);

    PROFILER_PUSH_BLOCK(context, "material-manager-prepare");
    //renderer_material_manager_prepare_frame(&context->materialManager, context->renderFrame);
    PROFILER_POP_BLOCK(context);

    PROFILER_PUSH_BLOCK(context, "scene-manager-render-frame");
    //renderer_scene_manager_render_frame(&context->sceneManager, context->renderFrame);
    PROFILER_POP_BLOCK(context);

    PROFILER_PUSH_BLOCK(context, "window-swap-buffers");
    window->swap_buffers(window);
    PROFILER_POP_BLOCK(context);

    frame_guard_release_complete_frame(&context->frameGuard);

    PROFILER_END_FRAME(context);
#ifdef STROBE_RENDERER_ENABLE_PROFILING
#ifdef STROBE_LOG_INFOS
    frametimeLogAcc +=
        frame_profiler_get_last_frametime(&context->frameProfiler);
    if (frametimeLogAcc >= 10ull * 1000000000ull) {
      logInfo(
          "[Renderer::Loop] Frametime Report:\n"
          "                 -average: %f ms\n"
          "                 -max:     %f ms\n"
          "                 -min:     %f ms\n",
          frame_profiler_average_frametime(&context->frameProfiler) /
              1000000.0f,
          frame_profiler_max_frametime(&context->frameProfiler) / 1000000.0f,
          frame_profiler_min_frametime(&context->frameProfiler) / 1000000.0f);
      frametimeLogAcc = 0;
    }
#endif
#endif
  }
  if (context->shouldClose) {
    logDebug("[Renderer::Loop] Shutdown event [origin = context] \n");
  } else {
    logDebug("[Renderer::Loop] Shutdown event [origin = window] \n");
  }
  logInfo("[Renderer::Loop] End\n");
  context->running = 0;

  logInfo("[Renderer::Shutdown] Begin\n");

  logInfo("[Renderer::Shutdown] Destroy frame guard\n");
  frame_guard_destroy(&context->frameGuard);

  logInfo("[Renderer::Shutdown]] Destroy scene manager\n");
  renderer_scene_manager_destroy(&context->sceneManager);

  // Destroy material manager
  logInfo("[Renderer::Shutdown] Destroy material manager\n");
  renderer_material_manager_destroy(&context->materialManager);

  // Destroy mesh manager.
  logInfo("[Renderer::Shutdown] Destroy mesh manager\n");
  renderer_mesh_manager_destroy(&context->meshManager);

#ifdef STROBE_RENDERER_ENABLE_PROFILING
  logInfo("[Renderer::Shutdown] Destroy profiler\n");
  frame_profiler_destroy(&context->frameProfiler);
#endif
  // Destroy backend.
  // TODO
  logInfo("[Renderer::Shutdown] Destroy render backend\n");

  // Destroy window.
  window->destroy(window);
  logInfo("[Renderer::Shutdown] Destroy window\n");

  // Signal destruction complete.
  frame_barrier_complete_wait(&context->frameBarrier);
  logInfo("[Renderer::Shutdown] End\n");

  logInfo("[Renderer] Exit with 0\n");

  return NULL;
}

void render_thread_start(render_context_t *context) {
  thread_create(&(context->renderThread), render_thread_entry, context);
  frame_barrier_submit_wait(&context->frameBarrier);
}

void render_thread_join(render_context_t *context) {
  frame_guard_release_submit_frame(&context->frameGuard);
  frame_barrier_submit_wait(&context->frameBarrier);
  thread_join(&context->renderThread, NULL);
}
