package org.aspect.graphics.window.api.glfw;

import org.aspect.exceptions.AspectWindowException;
import org.aspect.graphics.window.*;
import org.aspect.graphics.window.event.*;
import org.aspect.graphics.window.listener.WindowKeyListener;
import org.aspect.graphics.window.listener.WindowListener;
import org.aspect.graphics.window.listener.WindowMouseListener;
import org.lwjgl.glfw.*;
import org.lwjgl.opengl.GL;

import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.system.MemoryUtil.NULL;

public class GlfwWindow extends Window {

    private static boolean glfwInitialized = false;
    private final boolean vsync = false;
    private final boolean resizable = false;
    private long windowPointer;

    public GlfwWindow() {

    }

    private static void setupGlfw() {
        GLFWErrorCallback.createPrint(System.err).set();
        if (!glfwInit()) {
            throw new AspectWindowException(null, "failed to inialize glfw");
        }
    }

    @Override
    public void createWindow() {
        if (!glfwInitialized) setupGlfw();

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

        long monitor = NULL;
        if (isFullscreen()) {
            monitor = glfwGetPrimaryMonitor();
            GLFWVidMode vidMode = glfwGetVideoMode(monitor);
            setWidth(vidMode.width());
            setHeight(vidMode.height());
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

        windowPointer = glfwCreateWindow(getWidth(), getHeight(),
                getTitle(), monitor, NULL);

        if (windowPointer == NULL) {
            throw new AspectWindowException(this, "failed to create window");
        }

        glfwMakeContextCurrent(windowPointer);

        glfwSwapInterval(vsync ? GLFW_TRUE : GLFW_FALSE);

        GL.createCapabilities();
        glfwShowWindow(windowPointer);

    }

    @Override
    public void pollWindowEvents() {
        glfwPollEvents();
    }

    @Override
    public void swapBuffers() {
        glfwSwapBuffers(windowPointer);
    }

    @Override
    public void close() {
        glfwSetWindowShouldClose(windowPointer, true);
    }

    @Override
    public void dispose() {
        glfwDestroyWindow(windowPointer);
    }

    @Override
    public boolean shouldClose() {
        return glfwWindowShouldClose(windowPointer);
    }

    @Override
    public void connectWindowKeyListener(WindowKeyListener keyListener) {
        GLFWKeyCallback callback = new GLFWKeyCallback() {
            @Override
            public void invoke(long window, int key, int scancode, int action, int mods) {

                if(action == GLFW_PRESS){
                    keyListener.keyDown(new WindowKeyEvent(key, mods));
                }else if(action == GLFW_RELEASE){
                    keyListener.keyUp(new WindowKeyEvent(key, mods));
                }
            }
        };
        glfwSetKeyCallback(windowPointer, callback);
    }

    @Override
    public void connectWindowMouseListener(WindowMouseListener mouseListener) {
        GLFWScrollCallback scrollCallback = new GLFWScrollCallback() {
            @Override
            public void invoke(long window, double xoffset, double yoffset) {
                mouseListener.scroll(new WindowMouseScrollEvent(xoffset, yoffset));
            }
        };
        glfwSetScrollCallback(windowPointer, scrollCallback);

        GLFWMouseButtonCallback buttonCallback = new GLFWMouseButtonCallback() {
            @Override
            public void invoke(long window, int button, int action, int mods) {

                if(action == GLFW_PRESS){
                    mouseListener.mouseDown(new WindowMouseEvent(button, mods));
                }else if(action == GLFW_RELEASE){
                    mouseListener.mouseUp(new WindowMouseEvent(button, mods));
                }
            }
        };
        glfwSetMouseButtonCallback(windowPointer, buttonCallback);
    }

    @Override
    public void connectWindowListener(WindowListener windowListener) {
        windowListener.resize(new WindowResizeEvent(getWidth(), getHeight()));
        windowListener.focus(new WindowFocusEvent(true));
        glfwFocusWindow(windowPointer);
        int[] xpos = new int[1], ypos = new int[1];
        glfwGetWindowPos(windowPointer, xpos, ypos);
        windowListener.position(new WindowPositionEvent(xpos[0], ypos[0]));
        GLFWWindowSizeCallback sizeCallback = new GLFWWindowSizeCallback() {
            @Override
            public void invoke(long window, int width, int height) {
                windowListener.resize(new WindowResizeEvent(width, height));
            }
        };
        glfwSetWindowSizeCallback(windowPointer, sizeCallback);

        GLFWWindowFocusCallback focusCallback = new GLFWWindowFocusCallback() {
            @Override
            public void invoke(long window, boolean focused) {
                windowListener.focus(new WindowFocusEvent(focused));
            }
        };
        glfwSetWindowFocusCallback(windowPointer, focusCallback);

        GLFWWindowPosCallback posCallback = new GLFWWindowPosCallback() {
            @Override
            public void invoke(long window, int xpos, int ypos) {
                windowListener.position(new WindowPositionEvent(xpos, ypos));
            }
        };
        glfwSetWindowPosCallback(windowPointer, posCallback);
    }
}
