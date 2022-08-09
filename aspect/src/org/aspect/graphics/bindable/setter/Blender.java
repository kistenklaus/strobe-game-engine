package org.aspect.graphics.bindable.setter;

import static org.lwjgl.opengl.GL11.*;

public class Blender extends BindableSetter {

    @Override
    public void enable() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    @Override
    public void disable() {
        glDisable(GL_BLEND);
    }
}
