#include "GlfwWindow.h"
#include <utility>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <csignal>

// forward declaration.
void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar*
message, const void* userParam);

GlfwWindow::GlfwWindow(CreateInfo* createInfo) :
        m_width(createInfo->width),
        m_height(createInfo->height),
        m_title(std::move(createInfo->title)),
        m_window(nullptr),
        m_displayMode(createInfo->displayMode),
        m_resizable(createInfo->resizable),
        m_decorated(!createInfo->hideDecorations),
        m_floating(createInfo->alwaysOnTop),
        m_transparent(createInfo->transparentFramebuffer),
        m_doubleBuffering(createInfo->enableDoubleBuffering),
        m_redBits(createInfo->framebufferRedBits),
        m_greenBits(createInfo->framebufferGreenBits),
        m_blueBits(createInfo->framebufferBlueBits),
        m_depthBits(createInfo->framebufferDepthBits),
        m_stencilBits(createInfo->framebufferStencilBits) {
    assert(m_width > 0);
    assert(m_height > 0);
    assert(!m_title.empty());

    bool success = glfwInit();
    if (!success) {
        throw std::runtime_error("[Fatal Error] failed to initialize GLFW window Library");
    }

    GLFWmonitor *monitor = nullptr;
    switch (m_displayMode) {
        case Windowed:
            break;
        case Fullscreen:
            monitor = glfwGetPrimaryMonitor();
            break;
        case WindowedFullscreen:
            monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            break;
    }
    // OpenGL specific.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

    // Window Configuration
    glfwWindowHint(GLFW_RESIZABLE, m_resizable);
    glfwWindowHint(GLFW_DECORATED, m_decorated);
    glfwWindowHint(GLFW_FLOATING, m_floating);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, m_transparent);
    glfwWindowHint(GLFW_DOUBLEBUFFER, m_doubleBuffering);
    glfwWindowHint(GLFW_RED_BITS, m_redBits);
    glfwWindowHint(GLFW_GREEN_BITS, m_greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, m_blueBits);
    glfwWindowHint(GLFW_DEPTH_BITS, m_depthBits);
    glfwWindowHint(GLFW_STENCIL_BITS, m_stencilBits);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);


    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), monitor, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("[Fatal Error] Failed to create GLFW window!");
    }

    // Setup controller callbacks.

    // Create OpenGL context.
    glfwMakeContextCurrent(m_window);

    glfwSwapInterval(0);

    success = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    if (!success) {
        throw std::runtime_error("[Fatal Error] Failed to initialize GLAD");
    }

    // Create Debug Callback.
    int openGLContextFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &openGLContextFlags);
    if (openGLContextFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    }

    assert(m_window);
}


GlfwWindow::~GlfwWindow() {
    assert(m_window);
    glfwDestroyWindow(m_window);
}

bool GlfwWindow::shouldClose() {
    return glfwWindowShouldClose(m_window);
}

void GlfwWindow::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void GlfwWindow::pollEvents() {
    glfwPollEvents();
}

void GlfwWindow::close() {
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void GlfwWindow::setSize(const int width, const int height) {
    m_width = width;
    m_height = height;
    glfwSetWindowSize(m_window, width, height);
}

void GlfwWindow::setTitle(const std::string &title) {
    m_title = title;
    glfwSetWindowTitle(m_window, m_title.c_str());
}

int GlfwWindow::width() const { return m_width; }

int GlfwWindow::height() const { return m_height; }

std::string GlfwWindow::title() const { return m_title; }


void APIENTRY glDebugOutput(GLenum source,
                                         GLenum type,
                                         GLuint id,
                                         GLenum severity,
                                         GLsizei length,
                                         const GLchar *message,
                                         const void *userParam) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            std::cout << "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            std::cout << "Source: Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            std::cout << "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            std::cout << "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            std::cout << "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            std::cout << "Source: Other";
            break;
        default :
            break;
    }
    std::cout << std::endl;

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            std::cout << "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cout << "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cout << "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cout << "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cout << "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            std::cout << "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            std::cout << "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            std::cout << "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            std::cout << "Type: Other";
            break;
        default :
            break;
    }
    std::cout << std::endl;

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            std::cout << "Severity: high";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cout << "Severity: medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            std::cout << "Severity: low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            std::cout << "Severity: notification";
            break;
        default :
            break;
    }
    std::raise(SIGINT);

    std::cout << std::endl;
    std::cout << std::endl;
}
