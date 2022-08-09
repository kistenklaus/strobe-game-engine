package org.aspect.graphics.bindable;

import org.aspect.exceptions.AspectGraphicsException;
import org.aspect.exceptions.AspectInvalidUniform;
import org.joml.Matrix4f;
import org.joml.Vector4f;

import static org.lwjgl.opengl.GL11.GL_FALSE;
import static org.lwjgl.opengl.GL20.*;

public final class Shader extends Bindable {

    private int ID;
    private final String vertexSrc, fragmentSrc;

    private final String hash;

    public Shader(String vertexSrc, String fragmentSrc){
        hash = this.getClass()+vertexSrc+fragmentSrc;
        this.vertexSrc = vertexSrc;
        this.fragmentSrc = fragmentSrc;

    }

    public void create() {
        int vsID = compile(vertexSrc, GL_VERTEX_SHADER);
        int fsID = compile(fragmentSrc, GL_FRAGMENT_SHADER);
        int programID = link(vsID, fsID);
        detach(programID, vsID);
        detach(programID, fsID);
        ID = programID;
    }

    private int compile(String shaderSrc, int shaderType) {
        int id = glCreateShader(shaderType);
        glShaderSource(id, shaderSrc);
        glCompileShader(id);
        int success = glGetShaderi(id, GL_COMPILE_STATUS);
        if (success == GL_FALSE) {
            int len = glGetShaderi(id, GL_INFO_LOG_LENGTH);
            final String shader_name = "unknown";
            System.out.println("\u001B[33m" +
                    "failed to compile the " +
                    (shaderType == GL_VERTEX_SHADER ? "VERTEX_SHADER" : "FRAGMENT_SHADER") +
                    " of the Shader:" + shader_name + "\u001B[0m\"");
            System.out.println("\u001B[33m" + glGetShaderInfoLog(id, len) + "\u001B[0m\"");
            throw new AspectGraphicsException("GLSL ERROR");
        }
        return id;
    }

    private int link(int vsID, int fsID) {
        int id = glCreateProgram();
        glAttachShader(id, vsID);
        glAttachShader(id, fsID);
        glLinkProgram(id);
        int success = glGetProgrami(id, GL_LINK_STATUS);
        if (success == GL_FALSE) {
            int len = glGetProgrami(id, GL_INFO_LOG_LENGTH);
            System.out.println("\u001B[33m" + glGetShaderInfoLog(id, len) + "\u001B[0m\"");
            throw new AspectGraphicsException("ERROR: linking a shaderProgram failed!");
        }
        return id;
    }

    private void detach(int programID, int shaderID) {
        glDetachShader(programID, shaderID);
        glDeleteShader(shaderID);
    }

    @Override
    public void bind() {
        //System.out.println("Shader : bind");
        glUseProgram(ID);
    }

    @Override
    public void unbind() {
        glUseProgram(0);
    }

    @Override
    public void dispose() {
        glUseProgram(0);
        glDeleteProgram(ID);
    }

    /*
     ----------uniforms------------
     */

    public int queryUniform1i(String uniformName) {
        int loc = glGetUniformLocation(ID, uniformName);
        if (loc == -1) throw new AspectInvalidUniform("querying a uniform " + uniformName + " that doesn't exist");
        return glGetUniformi(ID, loc);
    }

    public int getUniformLocation(String uniformName) {
        return glGetUniformLocation(ID, uniformName);
    }

    public void uniformMat4(String uniformName, Matrix4f mat4) {
        int loc = glGetUniformLocation(ID, uniformName);
        if (loc != -1) {
            float[] mat4Array = new float[16];
            mat4.get(mat4Array);
            glUniformMatrix4fv(loc, false, mat4Array);
        }
    }

    public void uniformVec4(String uniformName, Vector4f vec4) {
        int loc = glGetUniformLocation(ID, uniformName);
        if (loc != -1) {
            glUniform4f(loc, vec4.x, vec4.y, vec4.z, vec4.w);
        }
    }

    public void uniform1i(String uniformName, int i) {
        int loc = glGetUniformLocation(ID, uniformName);
        if (loc != 1) {
            glUniform1i(loc, i);
        }
    }

    public void uniform1b(int loc, boolean value){
        if(loc != -1){
            glUniform1i(loc,value?1:0);
        }
    }

    public void uniform1b(String uniformName, boolean value){
        uniform1b(getUniformLocation(uniformName),value);
    }

    public String hash(){
        return hash;
    }
}
