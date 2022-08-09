package org.aspect.window.event;

public class WindowPositionEvent {

    private final int xpos, ypos;

    public WindowPositionEvent(int xpos, int ypos) {
        this.xpos = xpos;
        this.ypos = ypos;
    }

    public int getXPosition() {
        return xpos;
    }

    public int getYPosition() {
        return ypos;
    }
}
