package strobe.graphics.bindables;

import aspect.Aspect;
import aspect.AspectBindable;
import marshal.Marshal;
import org.joml.Matrix4f;
import strobe.StrobeException;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Arrays;

import static org.lwjgl.opengl.GL11.GL_FALSE;
import static org.lwjgl.opengl.GL20.*;
import static org.lwjgl.opengl.GL20.glGetProgramInfoLog;

public class Shader extends AspectBindable {

    private final String vertex_src, fragment_src;
    private final String shader_name;

    private int id;

    public Shader(String shader_path){
        this(shader_path+".vs", shader_path+".fs");
    }
    public Shader(String vertex_shader_path, String fragment_shader_path){
        this(vertex_shader_path, fragment_shader_path,
                "@:"+vertex_shader_path.split("/")[vertex_shader_path.split("/").length-1] +"&"+
                fragment_shader_path.split("/")[fragment_shader_path.split("/").length-1]);
    }
    public Shader(String vertex_shader_path, String fragment_shader_path, String shader_name){
        String temp_vertex_src = null, temp_fragment_src = null;
        try {
            temp_vertex_src = new String(Files.readAllBytes(Paths.get(vertex_shader_path)));
            temp_fragment_src = new String(Files.readAllBytes(Paths.get(fragment_shader_path)));
        } catch (IOException e) { e.printStackTrace(); Marshal.stop();}
        vertex_src = temp_vertex_src;
        fragment_src = temp_fragment_src;
        this.shader_name = shader_name;
    }

    @Override
    public void create() {
        System.out.println("create Shader");
        int vs_id = compile(vertex_src, GL_VERTEX_SHADER);
        int fs_id = compile(fragment_src, GL_FRAGMENT_SHADER);
        id = link(vs_id, fs_id);
        //detach(id, vs_id, fs_id);
    }

    private int compile(String shaderSrc, int shaderType) {
        int id = glCreateShader(shaderType);
        glShaderSource(id, shaderSrc);
        glCompileShader(id);
        int success = glGetShaderi(id, GL_COMPILE_STATUS);
        if (success == GL_FALSE) {
            int len = glGetShaderi(id, GL_INFO_LOG_LENGTH);
            System.out.println("\u001B[33m"+"failed to compile the "+(shaderType==GL_VERTEX_SHADER?"VERTEX_SHADER":"FRAGMENT_SHADER")+" of the Shader:"+shader_name+"\u001B[0m\"");
            System.out.println("\u001B[33m"+glGetShaderInfoLog(id, len)+"\u001B[0m\"");
            throw new StrobeException("GLSL ERROR");
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
            System.out.println("\u001B[33m"+glGetShaderInfoLog(id, len)+"\u001B[0m\"");
            throw new StrobeException("ERROR: linking a shaderProgram failed!");
        }
        return id;
    }

    private void detach(int id, int vs_id, int fs_id){
        glDetachShader(id, vs_id);
        glDeleteShader(vs_id);
        glDetachShader(id, fs_id);
        glDeleteShader(fs_id);
    }

    @Override
    public void bind() {
        glUseProgram(id);
    }

    @Override
    public void unbind() {
        glUseProgram(0);
    }

    @Override
    public void dispose() {
        glUseProgram(0);
        glDeleteProgram(id);
    }

    public UniformMapper getUniformMapper(String uniform_name){
        if(!Thread.currentThread().getName().equals(Aspect.getThreadName()))throw new IllegalAccessError();
        return new UniformMapper(glGetUniformLocation(id, uniform_name));
    }
    public class UniformMapper{
        private int loc;
        private UniformMapper(int location){
            this.loc = location;
        }
    }

    private void uniform1i(int loc, int integer){
        if(!Thread.currentThread().getName().equals(Aspect.getThreadName()))throw new IllegalAccessError();
        if(loc!=-1){
            glUniform1i(loc, integer);
        }
    }
    public void uniform1i(String uniform_name, int integer){
        uniform1i(glGetUniformLocation(id, uniform_name), integer);
    }
    public void uniform1i(UniformMapper uniform, int integer){
        uniform1i(uniform.loc, integer);
    }

    private void uniform1f(int loc, float floating){
        if(!Thread.currentThread().getName().equals(Aspect.getThreadName()))throw new IllegalAccessError();
        if(loc!=-1){
            glUniform1f(loc, floating);
        }
    }
    public void uniform1f(String uniform_name, float floating) {
        uniform1f(glGetUniformLocation(id, uniform_name), floating);
    }
    public void uniform1f(UniformMapper uniform, float floating){
        uniform1f(uniform.loc, floating);
    }

    private void uniformMat4(int loc, float[] mat4_arr){
        if(!Thread.currentThread().getName().equals(Aspect.getThreadName()))throw new IllegalAccessError();
        if(loc!=-1){
            glUniformMatrix4fv(loc, false, mat4_arr);
        }
    }
    private void uniformMat4(int loc, Matrix4f mat4){
        float[] mat4_arr = new float[16];
        mat4.get(mat4_arr);
        uniformMat4(loc, mat4_arr);
    }
    public void uniformMat4(String uniform_name, Matrix4f mat4){
        uniformMat4(glGetUniformLocation(id, uniform_name), mat4);
    }
    public void uniformMat4(String uniform_name, float[] mat4_arr){
        if(mat4_arr.length!=16)throw new IllegalArgumentException(Arrays.toString(mat4_arr));
        uniformMat4(glGetUniformLocation(id, uniform_name), mat4_arr);
    }
    public void uniformMat4(UniformMapper uniform, Matrix4f mat4){
        uniformMat4(uniform.loc, mat4);
    }
    public void uniformMat4(UniformMapper uniform, float[] mat4_arr){
        if(mat4_arr.length!=16)throw new IllegalArgumentException(Arrays.toString(mat4_arr));
        uniformMat4(uniform.loc, mat4_arr);
    }
}
