package org.strobe.window;

import java.util.ArrayList;
import java.util.List;

public final class WindowKeyboard implements WindowKeyListener {

    private final List<WindowKeyListener> listeners = new ArrayList<>();

    private final boolean[] keys = new boolean[WindowKey.values().length];

    WindowKeyboard() {

    }

    @Override
    public void keyDown(WindowKey key) {
        for (WindowKeyListener l : listeners) l.keyDown(key);
        keys[key.ordinal()] = true;
    }

    @Override
    public void keyUp(WindowKey key) {
        for (WindowKeyListener l : listeners) l.keyUp(key);
        keys[key.ordinal()] = false;
    }

    public boolean isKeyDown(WindowKey key) {
        return keys[key.ordinal()];
    }
}
