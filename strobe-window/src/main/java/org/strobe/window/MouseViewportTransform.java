package org.strobe.window;

import org.joml.Vector2f;

public final class MouseViewportTransform {


    private float viewportX;
    private float viewportY;
    private float viewportWidth;
    private float viewportHeight;

    public MouseViewportTransform(float viewportX, float viewportY, float viewportWidth, float viewportHeight){
        this.viewportX = viewportX;
        this.viewportY = viewportY;
        this.viewportWidth = viewportWidth;
        this.viewportHeight = viewportHeight;
    }

    public void updateViewport(float viewportX, float viewportY, float viewportWidth, float viewportHeight){
        this.viewportX = viewportX;
        this.viewportY = viewportY;
        this.viewportWidth = viewportWidth;
        this.viewportHeight = viewportHeight;
    }

    /**
     * transforms the mouse position to the placement of the viewport where the strobe context is rendered
     * @param mousex the mouse x position relative to the window
     * @param mousey the mouse y position relative to the window
     * @return the mouse position relative to the game context viewport in the window,
     * returns null if the mouse position is out of the viewport of the context.
     */
    public Vector2f transformMousePosition(float mousex, float mousey){
        //check bounding.

        return null;
    }
}
