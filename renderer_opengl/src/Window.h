#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window {
public:
    enum DisplayMode {
        Windowed,
        WindowedFullscreen,
        Fullscreen
    };
    struct CreateInfo {
        int width = 600;
        int height = 400;
        std::string title = "GLFW-Window";
        DisplayMode displayMode = Windowed;
        bool resizable = false;
        bool hideDecorations = false;
        bool alwaysOnTop = false;
        bool transparentFramebuffer = false;
        bool enableDoubleBuffering = true;
        int framebufferRedBits = 8;
        int framebufferGreenBits = 8;
        int framebufferBlueBits = 8;
        int framebufferAlphaBits = 8;
        int framebufferDepthBits = 24;
        int framebufferStencilBits = 8;
    };

    explicit Window(CreateInfo* createInfo);

    ~Window();

    bool shouldClose();

    void swapBuffers();

    static void pollEvents();

    void close();

    void setSize(int width, int height);

    void setTitle(const std::string &title);

    [[nodiscard]] int width() const;

    [[nodiscard]] int height() const;

    [[nodiscard]] std::string title() const;

private:
    int m_width;
    int m_height;
    std::string m_title;
    GLFWwindow *m_window;
    const DisplayMode m_displayMode;
    const bool m_resizable;
    const bool m_decorated;
    const bool m_floating;
    const bool m_transparent;
    const bool m_doubleBuffering;
    const int m_redBits = 8;
    const int m_greenBits = 8;
    const int m_blueBits = 8;
    const int m_depthBits = 24;
    const int m_stencilBits = 8;
};
