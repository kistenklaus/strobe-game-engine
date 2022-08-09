package org.aspect.window.listener;

import org.aspect.window.event.WindowKeyEvent;

public interface WindowKeyListener {

    void keyDown(WindowKeyEvent keyEvent);

    void keyUp(WindowKeyEvent keyEvent);

    default void onPoll() {

    }
}
