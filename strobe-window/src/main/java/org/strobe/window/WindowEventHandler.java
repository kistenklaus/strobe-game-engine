package org.strobe.window;

import java.util.ArrayList;
import java.util.List;

public class WindowEventHandler implements WindowEventListener {

    private final List<WindowEventListener> listeners = new ArrayList<>();

    WindowEventHandler() {

    }

    @Override
    public void onResize(int width, int height) {
        for (WindowEventListener l : listeners) l.onResize(width, height);
    }

    @Override
    public void onFocusChange(boolean focus) {
        for (WindowEventListener l : listeners) l.onFocusChange(focus);
    }

    @Override
    public void onPositionChange(int xpos, int ypos) {
        for (WindowEventListener l : listeners) l.onPositionChange(xpos, ypos);
    }

    public void addWindowEventListener(WindowEventListener listener){
        this.listeners.add(listener);
    }

    public boolean removeWindowEventListener(WindowEventListener listener){
        return this.listeners.remove(listener);
    }
}
