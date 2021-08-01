package org.strobe.window;

public interface WindowEventListener {

    void onResize(int width, int height);

    void onFocusChange(boolean focus);

    void onPositionChange(int xpos, int ypos);
}
