package org.strobe.window;

import java.util.ArrayList;
import java.util.List;

public final class WindowKeyboard implements WindowKeyListener {

    private final List<WindowKeyListener> listeners = new ArrayList<>();

    private WindowContentRegion contentRegion = null;

    private final boolean[] keys = new boolean[WindowKey.values().length];

    WindowKeyboard() {

    }

    @Override
    public void keyDown(WindowKey key) {
        keys[key.ordinal()] = true;
        if(contentRegion!=null && !contentRegion.isContentFocused())return;
        for (WindowKeyListener l : listeners) l.keyDown(key);
    }

    @Override
    public void keyUp(WindowKey key) {
        keys[key.ordinal()] = false;
        if(contentRegion != null && !contentRegion.isContentFocused())return;
        for (WindowKeyListener l : listeners) l.keyUp(key);
    }

    public boolean isKeyDown(WindowKey key) {
        if(contentRegion!=null && !contentRegion.isContentFocused())return false;
        return keys[key.ordinal()];
    }

    public void setContentRegion(WindowContentRegion contentRegion){
        this.contentRegion = contentRegion;
    }

    public void addKeyListener(WindowKeyListener keyListener){
        listeners.add(keyListener);
    }

    public boolean removeKeyListener(WindowKeyListener keyListener){
        return listeners.remove(keyListener);
    }
}
