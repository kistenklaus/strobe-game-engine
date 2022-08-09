package org.aspect.window.event;

public class WindowFocusEvent {

    private final boolean focus;

    public WindowFocusEvent(boolean focus) {
        this.focus = focus;
    }

    public boolean hasFocus(){
        return focus;
    }
}