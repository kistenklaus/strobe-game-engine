package org.strobe.gfx.opengl.bindables.shader;

import org.joml.Matrix4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;

@Pool(ShaderPool.class)
public class TShader extends Shader{

    private ShaderUniform<Matrix4f> transformUniform;

    public TShader(Graphics gfx, String vertexSrc, String fragmentSrc, String transformUniformName) {
        super(gfx, vertexSrc, fragmentSrc);
        transformUniform = getUniform(Matrix4f.class, transformUniformName);
    }

    public Uniform<Matrix4f> getTransformUniform(){
        return transformUniform;
    }
}
