package org.aspect.graphics.window.listener;

import org.aspect.graphics.window.event.WindowMouseEvent;
import org.aspect.graphics.window.event.WindowMouseScrollEvent;

public class WindowMouseHandler implements WindowMouseListener {

    private final boolean[] buttons = new boolean[WindowMouseEvent.MOUSE_BUTTON_LAST];

    private double deltaScrollX = 0;
    private double deltaScrollY = 0;

    @Override
    public void onPoll() {
        deltaScrollX = 0;
        deltaScrollY = 0;
    }

    @Override
    public void scroll(WindowMouseScrollEvent scrollEvent) {
        deltaScrollX+=scrollEvent.getDeltaScrollX();
        deltaScrollY+=scrollEvent.getDeltaScrollY();
    }

    @Override
    public void mouseDown(WindowMouseEvent mouseEvent) {
        buttons[mouseEvent.getButton()] = true;
        onMouseDown(mouseEvent);
    }

    public void onMouseDown(WindowMouseEvent mouseEvent){
    }

    @Override
    public void mouseUp(WindowMouseEvent mouseEvent) {
        buttons[mouseEvent.getButton()] = false;
        onMouseUp(mouseEvent);
    }

    public void onMouseUp(WindowMouseEvent mouseEvent){
    }

    public boolean isButtonDown(int button){
        return buttons[button];
    }

    public double getDeltaScrollX() {
        return deltaScrollX;
    }

    public double getDeltaScrollY() {
        return deltaScrollY;
    }
}
