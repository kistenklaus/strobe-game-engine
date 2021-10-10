package org.strobe.window;

import java.util.ArrayList;
import java.util.List;

public final class WindowMouse implements WindowMouseListener {

    private final List<WindowMouseListener> listeners = new ArrayList<>();

    //there variables will be updated by the Window directly
    private float windowWidth;
    private float windowHeight;

    private WindowContentRegion contentRegion = null;

    private double cxpos = 0, cypos = 0, lxpos = 0, lypos = 0;
    private double dxpos = 0, dypos = 0;
    private double cmx = 0, cmy = 0;
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
        xpos = (xpos / windowWidth) * 2 - 1;
        ypos = (ypos / windowHeight) * 2 - 1;

        if (contentRegion != null) {
            contentRegion.updateContentMousePos((float) xpos, (float) ypos);
            xpos = contentRegion.getRelativeMouseX();
            ypos = contentRegion.getRelativeMouseY();
        }

        for (WindowMouseListener l : listeners) l.onMove(xpos, ypos);
        cxpos = xpos;
        cypos = ypos;
        if (contentRegion == null || contentRegion.isContentFocused()) {
            cmx = cxpos;
            cmy = cypos;
        }
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
        if (contentRegion != null) contentRegion.updateContentMouseButton(true);
    }

    @Override
    public void onButtonUp(WindowButton button) {
        for (WindowMouseListener l : listeners) l.onButtonUp(button);
        buttons[button.ordinal()] = false;
        if (contentRegion != null) contentRegion.updateContentMouseButton(false);
    }

    protected void setContentRegion(WindowContentRegion viewportTransform) {
        this.contentRegion = viewportTransform;
    }

    public boolean isButtonDown(WindowButton button) {
        return buttons[button.ordinal()];
    }

    public double getX() {
        return cmy;
    }

    public double getY() {
        return cmx;
    }

    public double getDX() {
        if (contentRegion != null && !contentRegion.isContentFocused()) return 0;
        return dxpos;
    }

    public double getDY() {
        if (contentRegion != null && !contentRegion.isContentFocused()) return 0;
        return dypos;
    }

    public double getXScroll() {
        if(contentRegion != null && !contentRegion.isContentFocused())return 0;
        return cxscroll;
    }

    public double getYScroll() {
        if(contentRegion != null && !contentRegion.isContentFocused())return 0;
        return cyscroll;
    }

    protected void setWindowSize(float windowWidth, float windowHeight) {
        this.windowWidth = windowWidth;
        this.windowHeight = windowHeight;
    }

    public float getWindowAspect() {
        return windowWidth / windowHeight;
    }
}
