package org.aspect.graphics.renderer;

import static org.lwjgl.opengl.GL11.GL_LINES;
import static org.lwjgl.opengl.GL11.GL_TRIANGLES;

public enum DrawMode {
    TRIANGLES(GL_TRIANGLES), LINES(GL_LINES);

    private final int drawMode;

    DrawMode(int mode){
        this.drawMode = mode;
    }

    public int getDrawMode(){
        return drawMode;
    }
}
