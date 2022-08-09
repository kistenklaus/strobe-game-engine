package org.aspect.graphics.renderer;

import static org.lwjgl.opengl.GL11.*;

public enum BufferBits {
    DEPTH(GL_DEPTH_BUFFER_BIT),
    COLOR(GL_COLOR_BUFFER_BIT),
    STENCIL(GL_STENCIL_BUFFER_BIT);

    private final int glID;

    BufferBits(int glID){
        this.glID = glID;
    }

    public int getGlID(){
        return glID;
    }
}
