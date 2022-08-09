package org.aspect.graphics.window.event;

public class WindowResizeEvent extends WindowEvent{

    public final int width, height;

    public WindowResizeEvent(int width, int height) {
        this.width = width;
        this.height = height;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }
}
