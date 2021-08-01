package org.strobe.gfx.opengl.bindables.ubo;

import org.strobe.gfx.BindingMethod;
import org.strobe.gfx.Graphics;

import static org.lwjgl.opengl.GL30.glBindBufferBase;
import static org.lwjgl.opengl.GL31.GL_UNIFORM_BUFFER;

public class UboBindingMethod implements BindingMethod<Ubo> {

    private static final boolean SAVE_BINDING = true;
    private int[] bindings = new int[16];

    @Override
    public void bind(Graphics gfx, Ubo ubo) {
        ubo.bind(gfx);
        if(bindings[ubo.getBindingIndex()] != ubo.getID()){
            glBindBufferBase(GL_UNIFORM_BUFFER, ubo.getBindingIndex(), ubo.getID());
            bindings[ubo.getBindingIndex()] = ubo.getID();
        }
    }

    @Override
    public void unbind(Graphics gfx, Ubo ubo) {
        bindings[ubo.getBindingIndex()] = 0;
        if(SAVE_BINDING){
            glBindBufferBase(GL_UNIFORM_BUFFER, ubo.getBindingIndex(), 0);
        }
        ubo.unbind(gfx);
    }
}
