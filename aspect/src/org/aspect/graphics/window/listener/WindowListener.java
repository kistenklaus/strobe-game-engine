package org.aspect.graphics.window.listener;

import org.aspect.graphics.window.event.WindowFocusEvent;
import org.aspect.graphics.window.event.WindowPositionEvent;
import org.aspect.graphics.window.event.WindowResizeEvent;

public interface WindowListener {

    void resize(WindowResizeEvent resizeEvent);

    void focus(WindowFocusEvent focusEvent);

    void position(WindowPositionEvent positionEvent);

}
