package org.strobe.window;

import java.util.ArrayList;
import java.util.List;

public final class WindowMouse implements WindowMouseListener {

    private final List<WindowMouseListener> listeners = new ArrayList<>();

    private double cxpos = 0, cypos = 0, lxpos = 0, lypos = 0;
    private double dxpos = 0, dypos = 0;
    private double cxscroll = 0, cyscroll = 0;
    private final boolean[] buttons = new boolean[WindowButton.values().length];

    WindowMouse() {

    }

    public void beforePoll() {
        cxscroll = 0;
        cyscroll = 0;
    }

    public void afterPoll() {
        dxpos = lxpos - cxpos;
        dypos = lypos - cypos;
        lxpos = cxpos;
        lypos = cypos;
    }

    @Override
    public void onMove(double xpos, double ypos) {
        for (WindowMouseListener l : listeners) l.onMove(xpos, ypos);
        cxpos = xpos;
        cypos = ypos;
    }

    @Override
    public void onScroll(double xOffset, double yOffset) {
        for (WindowMouseListener l : listeners) l.onScroll(xOffset, yOffset);
        this.cxscroll = xOffset;
        this.cyscroll = yOffset;
    }

    @Override
    public void onButtonDown(WindowButton button) {
        for (WindowMouseListener l : listeners) l.onButtonDown(button);
        buttons[button.ordinal()] = true;
    }

    @Override
    public void onButtonUp(WindowButton button) {
        for (WindowMouseListener l : listeners) l.onButtonUp(button);
        buttons[button.ordinal()] = false;
    }

    public boolean isButtonDown(WindowButton button) {
        return buttons[button.ordinal()];
    }

    public double getX() {
        return cxpos;
    }

    public double getY() {
        return cypos;
    }

    public double getDX() {
        return dxpos;
    }

    public double getDY() {
        return dypos;
    }

    public double getXScroll() {
        return cxscroll;
    }

    public double getYScroll() {
        return cyscroll;
    }

}
