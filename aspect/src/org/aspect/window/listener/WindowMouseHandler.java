package org.aspect.window.listener;

import org.aspect.window.event.WindowMouseButtonEvent;
import org.aspect.window.event.WindowMousePositionEvent;
import org.aspect.window.event.WindowMouseScrollEvent;
import org.joml.Vector2f;

public class WindowMouseHandler implements WindowMouseListener {

    private final boolean[] buttons = new boolean[WindowMouseButtonEvent.MOUSE_BUTTON_LAST];

    private double deltaScrollX = 0;
    private double deltaScrollY = 0;

    private Vector2f position = new Vector2f();
    private Vector2f deltaPosition = new Vector2f();

    @Override
    public void onPoll() {
        deltaScrollX = 0;
        deltaScrollY = 0;
        deltaPosition.mul(0);
    }

    @Override
    public void scroll(WindowMouseScrollEvent scrollEvent) {
        deltaScrollX += scrollEvent.getDeltaScrollX();
        deltaScrollY += scrollEvent.getDeltaScrollY();
    }

    @Override
    public void mouseDown(WindowMouseButtonEvent mouseEvent) {
        buttons[mouseEvent.getButton()] = true;
        onMouseDown(mouseEvent);
    }

    public void onMouseDown(WindowMouseButtonEvent mouseEvent) {
    }

    @Override
    public void mouseUp(WindowMouseButtonEvent mouseEvent) {
        buttons[mouseEvent.getButton()] = false;
        onMouseUp(mouseEvent);
    }

    public void onMouseUp(WindowMouseButtonEvent mouseEvent) {
    }

    public void mouseMoved(WindowMousePositionEvent mouseEvent) {
        deltaPosition.add(new Vector2f(
                mouseEvent.getPosX() - position.x,
                mouseEvent.getPosY() - position.y
        ));
        position.set(mouseEvent.getPosX(), mouseEvent.getPosY());
        onMouseMoved(mouseEvent);
    }

    public void onMouseMoved(WindowMousePositionEvent mouseEvent) {
    }

    public boolean isButtonDown(int button) {
        return buttons[button];
    }

    public double getDeltaScrollX() {
        return deltaScrollX;
    }

    public double getDeltaScrollY() {
        return deltaScrollY;
    }

    public float getMouseX() {
        return position.x;
    }

    public float getMouseY() {
        return position.y;
    }

    public float getDeltaMouseX() {
        return deltaPosition.x;
    }

    public float getDeltaMouseY() {
        return deltaPosition.y;
    }
}
