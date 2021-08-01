package org.strobe.gfx.materials.shaders;

import org.joml.Matrix4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.shader.ShaderPool;

@Pool(ShaderPool.class)
public class RedTestShader extends MaterialShader{


    public RedTestShader(Graphics gfx) {
        super(gfx, "shaders/redTestShader");
    }

    @Override
    public void uniformModelMatrix(Graphics gfx, Matrix4f modelMatrix) {

    }
}
