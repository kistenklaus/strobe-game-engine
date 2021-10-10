package org.strobe.window;

public final class WindowContentRegion {


    /**
     * viewport dimensions and position in normal window space [-1,1]
     */
    private float viewportX;
    private float viewportY;
    private float viewportWidth;
    private float viewportHeight;

    private float tmx;
    private float tmy;

    private boolean viewportFocus = false;
    private boolean viewportContainsMouse = false;

    private int buttonsPressed = 0;

    public WindowContentRegion(float viewportX, float viewportY, float viewportWidth, float viewportHeight) {
        this.viewportX = viewportX;
        this.viewportY = viewportY;
        this.viewportWidth = viewportWidth;
        this.viewportHeight = viewportHeight;
    }

    public void updateContentRegion(float viewportX, float viewportY, float viewportWidth, float viewportHeight) {
        this.viewportX = viewportX;
        this.viewportY = viewportY;
        this.viewportWidth = viewportWidth;
        this.viewportHeight = viewportHeight;
    }

    public void updateContentMousePos(float smx, float smy) {
        tmx = (smx - viewportX) * 2.0f / viewportWidth - 1.0f;
        tmy = (smy - viewportY) * 2.0f / viewportHeight - 1.0f;

        viewportContainsMouse = Math.abs(tmx) <= 1.0f && Math.abs(tmy) <= 1.0f;
    }

    public void updateContentMouseButton(boolean pressed) {
        if(pressed)buttonsPressed++;
        else buttonsPressed--;

        if (buttonsPressed == 1 && pressed && viewportContainsMouse)
            viewportFocus = true;
        if (buttonsPressed == 1 && pressed && !viewportContainsMouse)
            viewportFocus = false;
    }

    public boolean isContentFocused() {
        return viewportFocus;
    }

    public float getRelativeMouseX() {
        return tmx;
    }

    public float getRelativeMouseY() {
        return tmy;
    }
}
