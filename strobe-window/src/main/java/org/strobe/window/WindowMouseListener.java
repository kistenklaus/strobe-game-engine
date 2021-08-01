package org.strobe.window;

public interface WindowMouseListener {

    void onMove(double xpos, double ypos);

    void onScroll(double xOffset, double yOffset);

    void onButtonDown(WindowButton button);

    void onButtonUp(WindowButton button);

}
