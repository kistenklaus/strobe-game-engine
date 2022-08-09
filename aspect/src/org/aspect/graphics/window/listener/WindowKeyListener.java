package org.aspect.graphics.window.listener;

import org.aspect.graphics.window.event.WindowKeyEvent;

public interface WindowKeyListener {

    void keyDown(WindowKeyEvent keyEvent);

    void keyUp(WindowKeyEvent keyEvent);

    default void onPoll() {

    }
}
