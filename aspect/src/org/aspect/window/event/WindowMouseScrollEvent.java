package org.aspect.window.event;

public final class WindowMouseScrollEvent extends WindowEvent {


    private final double deltaScrollX, deltaScrollY;

    public WindowMouseScrollEvent(double deltaScrollX, double deltaScrollY) {
        this.deltaScrollX = deltaScrollX;
        this.deltaScrollY = deltaScrollY;
    }

    public double getDeltaScrollX() {
        return deltaScrollX;
    }

    public double getDeltaScrollY() {
        return deltaScrollY;
    }
}
