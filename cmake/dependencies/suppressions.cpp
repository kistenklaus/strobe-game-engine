// sanitizer_runtime.cpp

#if !defined(NDEBUG)

extern "C" [[gnu::used, gnu::visibility("default")]]
const char *__asan_default_options() {
  return "protect_shadow_gap=0";
}

extern "C" [[gnu::used, gnu::visibility("default")]]
const char *__lsan_default_options() {
  return "print_suppressions=0";
}

extern "C" [[gnu::used, gnu::visibility("default")]]
const char *__lsan_default_suppressions() {
  return R"(
leak:libnvidia-glcore.so
leak:libcuda.so
leak:libdbus-1.so
leak:glfwInit
leak:vkCreateInstance
leak:pthread_once
)";
}

#endif
