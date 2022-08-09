package org.aspect.window.event;

public class WindowMousePositionEvent extends WindowEvent{

    public float posX, posY;

    public WindowMousePositionEvent(float posX, float posY){
        this.posX = posX;
        this.posY = posY;
    }

    public float getPosX() {
        return posX;
    }

    public float getPosY() {
        return posY;
    }
}
