package org.strobe.gfx.materials.shaders;

import org.joml.Matrix4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.shader.ShaderPool;

@Pool(ShaderPool.class)
public final class GreenTestShader extends MaterialShader{

    public GreenTestShader(Graphics gfx) {
        super(gfx, "shaders/testShader");
    }

    @Override
    public void uniformModelMatrix(Graphics gfx, Matrix4f modelMatrix) {
        //for testing reasons empty
    }
}
