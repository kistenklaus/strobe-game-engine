package org.aspect.graphics.window;

import org.aspect.graphics.window.api.glfw.GlfwWindow;

import java.util.function.Supplier;

public enum WindowAPI {
    GLFW(GlfwWindow::new);

    Supplier<Window> constructor;

    WindowAPI(Supplier<Window> constructor) {
        this.constructor = constructor;
    }

    public Window newWindow() {
        return this.constructor.get();
    }
}
