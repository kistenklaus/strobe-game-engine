package org.aspect.graphics.bindable.setter;

import static org.lwjgl.opengl.GL11.*;

public class DepthTester extends BindableSetter {


    @Override
    public void enable() {
        glEnable(GL_DEPTH_TEST);
    }

    @Override
    public void disable() {
        glDisable(GL_DEPTH_TEST);
    }
}
