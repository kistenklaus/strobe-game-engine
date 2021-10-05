package org.strobe.gfx.materials.shaders;

import org.joml.Matrix4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.materials.MaterialShaderCompileConstantSet;
import org.strobe.gfx.opengl.bindables.shader.Shader;
import org.strobe.gfx.opengl.bindables.shader.ShaderPool;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.utils.ResourceLoader;


@Pool(ShaderPool.class)
public abstract class MaterialShader extends Shader {

    public MaterialShader(Graphics gfx, String shaderDirectory) {
        super(gfx, ResourceLoader.readContent(shaderDirectory + "/vertex.vs"),
                ResourceLoader.readContent(shaderDirectory + "/fragment.frag"),
                MaterialShaderCompileConstantSet.getInstance());
    }

    public abstract void uniformModelMatrix(Graphics gfx, Matrix4f modelMatrix);

    public void uniformShadowMap(Graphics gfx, Texture2D shadowMap) {
    }

    @Override
    public String toString() {
        return "Material-"+super.toString();
    }
}
