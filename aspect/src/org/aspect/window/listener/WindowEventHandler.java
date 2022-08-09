package org.aspect.window.listener;

import org.aspect.window.event.WindowFocusEvent;
import org.aspect.window.event.WindowPositionEvent;
import org.aspect.window.event.WindowResizeEvent;

public class WindowEventHandler implements WindowListener {

    private int windowWidth, windowHeight;
    private boolean windowFocus;
    private int windowXPos, windowYPos;


    @Override
    public final void resize(WindowResizeEvent resizeEvent) {
        windowWidth = resizeEvent.getWidth();
        windowHeight = resizeEvent.getHeight();
        onResize(resizeEvent);
    }

    public void onResize(WindowResizeEvent resizeEvent) {
    }

    @Override
    public final void focus(WindowFocusEvent focusEvent) {
        windowFocus = focusEvent.hasFocus();
        onFocus(focusEvent);
    }

    public void onFocus(WindowFocusEvent focusEvent) {
    }

    @Override
    public final void position(WindowPositionEvent positionEvent) {
        windowXPos = positionEvent.getXPosition();
        windowYPos = positionEvent.getYPosition();
        onPosition(positionEvent);
    }

    public void onPosition(WindowPositionEvent positionEvent) {
    }

    public int getWindowWidth() {
        return windowWidth;
    }

    public int getWindowHeight() {
        return windowHeight;
    }

    public boolean windowHasFocus() {
        return windowFocus;
    }

    public int getWindowXPos() {
        return windowXPos;
    }

    public int getWindowYPos() {
        return windowYPos;
    }
}
