package org.strobe.gfx.opengl.bindables.shader.io;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.shader.Shader;
import org.strobe.utils.ResourceLoader;

public final class ShaderLoader {

    private static final String DEFAULT_VERTEX_SUFFIX = ".vs";
    private static final String DEFAULT_FRAGMENT_SUFFIX = ".frag";

    private static final String DEFAULT_VERTEX_NAME = "vertex"+DEFAULT_VERTEX_SUFFIX;
    private static final String DEFAULT_FRAGMENT_NAME = "fragment"+DEFAULT_FRAGMENT_SUFFIX;


    public static Shader loadShader(Graphics gfx, String shaderResourceHint) {
        if(shaderResourceHint.endsWith("/")){
            return loadShader(gfx,
                    shaderResourceHint+DEFAULT_VERTEX_NAME,
                    shaderResourceHint+DEFAULT_FRAGMENT_NAME);
        }else{
            return loadShader(gfx,
                    shaderResourceHint+DEFAULT_VERTEX_SUFFIX,
                    shaderResourceHint+DEFAULT_FRAGMENT_SUFFIX);
        }
    }
    public static Shader loadShader(Graphics gfx, String vertexResource, String fragmentResource){
        return new Shader(gfx,
                ResourceLoader.readContent(vertexResource),
                ResourceLoader.readContent(fragmentResource));
    }

}