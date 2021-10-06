package org.strobe.window;

import org.joml.Vector2f;
import org.joml.Vector2i;

import java.util.ArrayList;
import java.util.List;

public final class WindowMouse implements WindowMouseListener {

    private final List<WindowMouseListener> listeners = new ArrayList<>();

    //there variables will be updated by the Window directly
    private float windowWidth;
    private float windowHeight;

    private MouseViewportTransform viewportTransform = null;

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
        //normalize mouse coordinates
        xpos = (xpos/windowWidth)*2+1;
        ypos = (ypos/windowHeight)*2+1;

        if(viewportTransform != null){
            Vector2f tpos = viewportTransform.transformMousePosition((float)xpos, (float)ypos);
            if(tpos == null)return;
            xpos = tpos.x;
            ypos = tpos.y;
        }

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

    public void setViewportTransform(MouseViewportTransform viewportTransform){
        this.viewportTransform = viewportTransform;
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

    protected void setWindowSize(float windowWidth, float windowHeight){
        this.windowWidth = windowWidth;
        this.windowHeight = windowHeight;
    }

    public float getWindowAspect() {
        return windowWidth/windowHeight;
    }
}
