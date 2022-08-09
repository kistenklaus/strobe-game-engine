package org.aspect.graphics.renderer;

import static org.lwjgl.opengl.GL11.*;

public enum CullFace {
    NONE(false, 0), BACK(true, GL_BACK), FRONT(true, GL_FRONT), FRONT_AND_BACK(true, GL_FRONT_AND_BACK);

    private int face_glID;
    private boolean culling;

    CullFace(boolean culling, int glID) {
        this.culling = culling;
        this.face_glID = glID;
    }

    public int getFaceGlID() {
        return face_glID;
    }

    public boolean isCulling() {
        return culling;
    }
}
