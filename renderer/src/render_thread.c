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

typedef void (*destroyCall)(render_context_t* context);

static void destroy_window(render_context_t* context){
  logInfo("[Renderer::Shutdown ] Destroy window\n");
  context->window->destroy(context->window);
}

static void destroy_render_backend(render_context_t* context){
  logInfo("[Renderer::Shutdown ] Destroy render backend\n");
  context->backend.destroy(&context->backend);
}

#ifdef STROBE_RENDERER_ENABLE_PROFILING
static void destroy_profiler(render_context_t* context){
  logInfo("[Renderer::Shutdown ] Destroy profiler\n");
  frame_profiler_destroy(&context->frameProfiler);
}
#endif

static void destroy_mesh_manager(render_context_t* context){
  logInfo("[Renderer::Shutdown ] Destroy mesh manager\n");
  renderer_mesh_manager_destroy(&context->meshManager);
}
static void destroy_material_manager(render_context_t* context){
  logInfo("[Renderer::Shutdown ] Destroy material manager\n");
  renderer_material_manager_destroy(&context->materialManager);
}

static void destroy_scene_manager(render_context_t* context){
  logInfo("[Renderer::Shutdown ] Destroy scene manager\n");
  renderer_scene_manager_destroy(&context->sceneManager);
}

static void destroy_frame_guard(render_context_t* context){
  logInfo("[Renderer::Shutdown ] Destroy frame guard\n");
  frame_guard_destroy(&context->frameGuard);
}

static void skipInitalizationGotoShutdown(render_context_t* context){
  frame_barrier_complete_wait(&context->frameBarrier);
  context->running = 0;
  context->shouldClose = 1;
}

#define SKIP_INITALIZATION(context) {\
  skipInitalizationGotoShutdown(context); \
  goto shutdown_label;}\

static void *render_thread_entry(void *context_arg) {
  render_context_t *context = context_arg;
  darray_create(&context->requiredDestroyCalls, sizeof(destroyCall), 8);

  logInfo("[Renderer::Setup    ] Begin\n");
  // Initalize window.
  logInfo("[Renderer::Setup    ] Initalize window\n");
  window_t *window = context->window;
  if(window->init(window)){
    logError("[Renderer::Setup    ] Failed to initalize window\n");
    SKIP_INITALIZATION(context);
  }
  logDebug("[Renderer::Setup    ] WindowInfo: \n"
                ".....Window-Dimensions : %u x %u\n"
                ".....Window-Title      : %s\n",
           window->title);
  *(destroyCall*)darray_emplace_back(&context->requiredDestroyCalls) = destroy_window;

  unsigned int frameCount = context->frameGuard.frameCount;

  // Initalize backend
  logInfo("[Renderer::Setup    ] Initalize render backend\n");
  render_backend *backend = &context->backend;
  if(backend->init(backend, &frameCount, context->window)){
    logError("[Renderer::Setup    ] Failed to inialize render backend\n");
    SKIP_INITALIZATION(context)
  }
  *(destroyCall*)darray_emplace_back(&context->requiredDestroyCalls) = destroy_render_backend;

#ifdef STROBE_RENDERER_ENABLE_PROFILING
  logInfo("[Renderer::Setup    ] Initalize profiler\n");
  frame_profiler_create_info frameProfilerCreateInfo;
  frameProfilerCreateInfo.historySize = 128;
  frame_profiler_init(&context->frameProfiler, &frameProfilerCreateInfo);
  *(destroyCall*)darray_emplace_back(&context->requiredDestroyCalls) = destroy_profiler;
#endif

  // Initalize mesh manager
  logInfo("[Renderer::Setup    ] Initalize mesh manager\n");
  mesh_manager_create_info meshManagerCreateInfo;
  meshManagerCreateInfo.frameCount = frameCount;
  if(renderer_mesh_manager_init(&context->meshManager, &meshManagerCreateInfo)){
    logError("[Renderer::Setup    ] Failed to initalize mesh manager\n");
    SKIP_INITALIZATION(context);
  }
  *(destroyCall*)darray_emplace_back(&context->requiredDestroyCalls) = destroy_mesh_manager;

  // Initalize material manager
  logInfo("[Renderer::Setup    ] Initalize material manager\n");
  material_manager_create_info materialManagerCreateInfo;
  materialManagerCreateInfo.frameCount = frameCount;
  if(renderer_material_manager_init(&context->materialManager,
                                 &materialManagerCreateInfo)){
    logError("[Renderer::Setup    ] Failed to initalize material manager\n");
    SKIP_INITALIZATION(context);
  }
  *(destroyCall*)darray_emplace_back(&context->requiredDestroyCalls) = destroy_material_manager;

  logInfo("[Renderer::Setup    ] Initalize scene manager\n");
  scene_manager_create_info sceneManagerCreateInfo;
  sceneManagerCreateInfo.frameCount = frameCount;
#ifdef STROBE_RENDERER_ENABLE_PROFILING
  sceneManagerCreateInfo.frameProfiler = &context->frameProfiler;
#endif
  renderer_scene_manager_init(&context->sceneManager, &sceneManagerCreateInfo);
  *(destroyCall*)darray_emplace_back(&context->requiredDestroyCalls) = destroy_scene_manager;

  // Initalize frame guard.
  logInfo("[Renderer::Setup    ] Initalize frame guard\n");
  frame_guard_init(&context->frameGuard, frameCount);
  *(destroyCall*)darray_emplace_back(&context->requiredDestroyCalls) = destroy_frame_guard;

  context->running = 1;
  // Signal initalize complete.
  frame_barrier_complete_wait(&context->frameBarrier);

  logInfo("[Renderer::Setup    ] End\n");

#ifdef STROBE_RENDERER_ENABLE_PROFILING
#ifdef STROBE_LOG_INFOS
  unsigned long long frametimeLogAcc = 7ull * 1000000000ull;
#endif
#endif

  logInfo("[Renderer::Loop     ] Begin\n");
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
          "[Renderer::Loop     ] Frametime Report:\n"
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
    logDebug("[Renderer::Loop     ] Shutdown event [origin = context] \n");
  } else {
    logDebug("[Renderer::Loop     ] Shutdown event [origin = window] \n");
  }
  logInfo("[Renderer::Loop     ] End\n");
  frame_guard_release_complete_frame(&context->frameGuard);
shutdown_label:
  context->running = 0;
  logInfo("[Renderer::Shutdown ] Begin\n");
  logInfo("[Renderer::Shutdown ] Synchronize shutdown\n");
  frame_barrier_complete_wait(&context->frameBarrier);
  for(int i = darray_size(&context->requiredDestroyCalls) - 1; 
      i >= 0; i--){
    (*(destroyCall*)darray_get(&context->requiredDestroyCalls, i))(context);
  }
  darray_destroy(&context->requiredDestroyCalls);

  // Signal destruction complete.
  //frame_barrier_complete_wait(&context->frameBarrier);
  logInfo("[Renderer::Shutdown ] End\n");

  logInfo("[Renderer::Thread   ] Exit with 0\n");

  return NULL;
}

int render_thread_start(render_context_t *context) {
  thread_create(&(context->renderThread), render_thread_entry, context);
  frame_barrier_submit_wait(&context->frameBarrier);
  if(!context->running){
    render_thread_join(context);
    return 1;
  }else{
    return 0;
  }
}

void render_thread_join(render_context_t *context) {
  logInfo("[Renderer::Interface] Synchronize shutdown\n");
  if(context->running){
    frame_guard_release_submit_frame(&context->frameGuard);
  }
  frame_barrier_submit_wait(&context->frameBarrier);
  logInfo("[Renderer::Interface] joining render thread\n");
  thread_join(&context->renderThread, NULL);
  logInfo("[Renderer] shutdown complete\n");
}
