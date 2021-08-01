package org.strobe.gfx.opengl.bindables.shader;

import static org.lwjgl.opengl.GL20.GL_FRAGMENT_SHADER;
import static org.lwjgl.opengl.GL20.GL_VERTEX_SHADER;
import static org.lwjgl.opengl.GL43.GL_PROGRAM;

public class GLSLCompileException extends RuntimeException{

    public GLSLCompileException(int shaderType, String msg){
        super("\u001B[33m failed to compile : " + format(shaderType) + "\n" + msg + "\u001B[0m\"");
    }

    private static String format(int type){
        return switch(type){
            case GL_VERTEX_SHADER -> "VERTEX_SHADER";
            case GL_FRAGMENT_SHADER -> "FRAGMENT_SHADER";
            case GL_PROGRAM -> "SHADER_PROGRAM";
            default -> throw new IllegalStateException();
        };
    }

}
