package org.aspect.graphics.window.listener;

import org.aspect.graphics.window.event.WindowMouseEvent;
import org.aspect.graphics.window.event.WindowMouseScrollEvent;

public interface WindowMouseListener {

    void scroll(WindowMouseScrollEvent scrollEvent);

    void mouseDown(WindowMouseEvent mouseEvent);

    void mouseUp(WindowMouseEvent mouseEvent);

    default void onPoll(){
    }
}
