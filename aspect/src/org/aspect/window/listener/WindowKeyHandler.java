package org.aspect.window.listener;

import org.aspect.window.event.WindowKeyEvent;

public class WindowKeyHandler implements WindowKeyListener {

    private boolean[] keys = new boolean[WindowKeyEvent.KEY_LAST];

    public WindowKeyHandler(){

    }

    public final void onPoll(){

    }

    @Override
    public final void keyDown(WindowKeyEvent keyEvent) {
        keys[keyEvent.getKey()] = true;
        onKeyDown(keyEvent);
    }

    public void onKeyDown(WindowKeyEvent keyEvent) {
    }

    @Override
    public final void keyUp(WindowKeyEvent keyEvent) {
        keys[keyEvent.getKey()] = false;
        onKeyUp(keyEvent);
    }

    public void onKeyUp(WindowKeyEvent keyEvent) {
    }

    public boolean isKeyDown(int key){
        return keys[key];
    }
}
