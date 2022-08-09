package org.aspect.window.listener;

import org.aspect.window.event.WindowFocusEvent;
import org.aspect.window.event.WindowPositionEvent;
import org.aspect.window.event.WindowResizeEvent;

public interface WindowListener {

    void resize(WindowResizeEvent resizeEvent);

    void focus(WindowFocusEvent focusEvent);

    void position(WindowPositionEvent positionEvent);

}
