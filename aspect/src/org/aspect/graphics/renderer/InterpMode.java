package org.aspect.graphics.renderer;

import static org.lwjgl.opengl.GL11.GL_LINEAR;
import static org.lwjgl.opengl.GL11.GL_NEAREST;

public enum InterpMode {
    LINEAR(GL_LINEAR), NEAREST(GL_NEAREST);

    private final int glID;

    InterpMode(int glID) {
        this.glID = glID;
    }

    public int getGlID() {
        return glID;
    }
}
