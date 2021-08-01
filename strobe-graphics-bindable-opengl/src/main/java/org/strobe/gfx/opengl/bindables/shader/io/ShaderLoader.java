package org.strobe.gfx.opengl.bindables.shader.io;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.shader.Shader;
import org.strobe.gfx.opengl.bindables.shader.TShader;
import org.strobe.io.ResourceLoader;

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

    public static TShader loadTShader(Graphics gfx, String shaderResourceHint, String transformUniformName){
        if(shaderResourceHint.endsWith("/")){
            return loadTShader(gfx,
                    shaderResourceHint+DEFAULT_VERTEX_NAME,
                    shaderResourceHint+DEFAULT_FRAGMENT_NAME,
                    transformUniformName);
        }else{
            return loadTShader(gfx,
                    shaderResourceHint+DEFAULT_VERTEX_SUFFIX,
                    shaderResourceHint+DEFAULT_FRAGMENT_SUFFIX,
                    transformUniformName);
        }
    }

    public static TShader loadTShader(Graphics gfx, String vertexResource,
                                      String fragmentResource, String transformUniformName){
        return new TShader(gfx,
                ResourceLoader.readContent(vertexResource),
                ResourceLoader.readContent(fragmentResource),
                transformUniformName);
    }

    public static Shader loadShader(Graphics gfx, String vertexResource, String fragmentResource){
        return new Shader(gfx,
                ResourceLoader.readContent(vertexResource),
                ResourceLoader.readContent(fragmentResource));
    }

}