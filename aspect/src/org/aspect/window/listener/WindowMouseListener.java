package org.aspect.window.listener;

import org.aspect.window.event.WindowMouseButtonEvent;
import org.aspect.window.event.WindowMousePositionEvent;
import org.aspect.window.event.WindowMouseScrollEvent;

public interface WindowMouseListener {

    void scroll(WindowMouseScrollEvent scrollEvent);

    void mouseDown(WindowMouseButtonEvent mouseEvent);

    void mouseUp(WindowMouseButtonEvent mouseEvent);

    void mouseMoved(WindowMousePositionEvent mouseEvent);

    default void onPoll(){
    }
}
