package org.strobe.gfx.opengl.bindables.shader;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.ubo.Ubo;
import org.strobe.gfx.opengl.bindables.util.TypeUtil;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringReader;
import java.util.*;

import static org.lwjgl.opengl.GL11.GL_FALSE;
import static org.lwjgl.opengl.GL20.*;
import static org.lwjgl.opengl.GL43.GL_PROGRAM;

@Pool(ShaderPool.class)
public class Shader extends Bindable<ShaderPool> {

    protected final int ID;
    private final Ubo[] ubos;
    private final Map<String, ShaderUniform> uniforms;

    private boolean bound = false;

    //for Sampler Uniforms
    protected int samplerCount = 0;

    public Shader(Graphics gfx, String vertexSrc, String fragmentSrc) {
        super(gfx);
        Shader pooledShader = pool.getShader(vertexSrc, fragmentSrc);
        if (pooledShader == null) {
            int vsID = compile(vertexSrc, GL_VERTEX_SHADER);
            int fsID = compile(fragmentSrc, GL_FRAGMENT_SHADER);
            ID = link(vsID, fsID);
            detach(ID, vsID);
            detach(ID, fsID);
            pool.addShader(vertexSrc, fragmentSrc, this);
            this.ubos = parseProgramUbos(gfx, vertexSrc, fragmentSrc);
            this.uniforms = parseProgramUniformSet(gfx, this, vertexSrc, fragmentSrc);
        } else {
            ID = pooledShader.ID;
            ubos = pooledShader.ubos;
            uniforms = pooledShader.uniforms;
        }
    }

    public Ubo getUbo(int binding) {
        for (Ubo ubo : ubos) {
            if (ubo.getBindingIndex() == binding) return ubo;
        }
        return null;
    }

    public <T> ShaderUniform<T> getUniform(Class<T> type, String name) {
        ShaderUniform uniform = uniforms.get(name);
        if (uniform == null) throw new IllegalArgumentException("shader doesn't have a uniform " + name);
        try {
            return (ShaderUniform<T>) uniform;
        } catch (ClassCastException e) {
            throw new IllegalArgumentException("uniform " + name + " is not of type " + type);
        }
    }

    public ShaderUniformSet createShaderUniformSet(Graphics gfx) {
        return new ShaderUniformSet(gfx, uniforms);
    }

    @Override
    protected void bind(Graphics gfx) {
        glUseProgram(ID);
        if (uniforms != null) {
            for (ShaderUniform uniform : uniforms.values()) {
                uniform.onShaderBind(gfx);
            }
        }
    }

    @Override
    protected void unbind(Graphics gfx) {
        if (uniforms != null) {
            for (ShaderUniform uniform : uniforms.values()) {
                uniform.onShaderUnbind(gfx);
            }
        }
        glUseProgram(0);
    }

    @Override
    protected void dispose(Graphics gfx) {
        glUseProgram(0);
        glDeleteProgram(ID);
    }


    private int compile(String shaderSrc, int shaderType) {
        int id = glCreateShader(shaderType);
        glShaderSource(id, shaderSrc);
        glCompileShader(id);
        int success = glGetShaderi(id, GL_COMPILE_STATUS);
        if (success == GL_FALSE) {
            int len = glGetShaderi(id, GL_INFO_LOG_LENGTH);
            throw new GLSLCompileException(shaderType, glGetShaderInfoLog(id, len));
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
            throw new GLSLCompileException(GL_PROGRAM, glGetShaderInfoLog(id, len));
        }
        return id;
    }

    private void detach(int programID, int shaderID) {
        glDetachShader(programID, shaderID);
        glDeleteShader(shaderID);
    }

    @Override
    public String toString() {
        return getClass().getSimpleName()+"{" +
                "ID=" + ID +
                '}';
    }

    private static Map<String, ShaderUniform> parseProgramUniformSet(Graphics gfx, Shader shader, String vertexSrc, String fragmentSrc) {
        HashMap<String, ShaderUniform> uniforms = new HashMap<>();
        parseShaderUniformSet(gfx, shader, GL_VERTEX_SHADER, vertexSrc, uniforms);
        parseShaderUniformSet(gfx, shader, GL_FRAGMENT_SHADER, fragmentSrc, uniforms);
        return uniforms;
    }

    private static void parseShaderUniformSet(Graphics gfx, Shader shader, int type, String src, Map<String, ShaderUniform> uniforms) {
        try {
            BufferedReader br = new BufferedReader(new StringReader(src));
            String line;
            boolean inUboBlock = false;
            ArrayList<String> structure = null;
            Map<String, ArrayList<String>> structs = new HashMap<>();
            while ((line = br.readLine()) != null) {
                line = line.trim();
                if (line.startsWith("layout(std140")) {
                    inUboBlock = true;
                }
                if (inUboBlock && line.contains("}")) {
                    inUboBlock = false;
                }
                if (line.startsWith("struct")) {
                    String[] split = line.split(" ");
                    structure = new ArrayList<>();
                    String structName = split[1]
                            .replace("{", "")
                            .replace("}", "")
                            .replace(" ", "");
                    structs.put(structName, structure);
                }
                if (structure != null) {
                    if(line.matches("[a-z]+[1-9]* [a-z|A-Z|0-9]+;")){
                        structure.add(line);
                    }
                }
                if (structure != null && line.contains("}")) {
                    structure = null;
                }
                if (!inUboBlock && structure == null && line.startsWith("uniform")) {
                    String rest = line.substring("uniform".length() + 1, line.length() - 1);
                    String varType = rest.substring(0, rest.indexOf(" "));
                    String name = rest.substring(rest.lastIndexOf(" ") + 1);
                    if(structs.containsKey(varType)){
                        for(String structLine : structs.get(varType)){
                            String[] split = structLine.split(" ");
                            String attribType = split[0];
                            String attribName = split[1].substring(0, split[1].indexOf(";"));
                            Class varClass = (Class) TypeUtil.glslTypeToClass(attribType);
                            ShaderUniform uniform = ShaderUniformLoader.getShaderUniformFor(gfx,
                                    shader, varClass, name+"."+attribName);
                            uniforms.put(name+"."+attribName, uniform);
                        }
                    }else{
                        Class<?> varClass = (Class<?>) TypeUtil.glslTypeToClass(varType);
                        ShaderUniform uniform = ShaderUniformLoader.getShaderUniformFor(gfx, shader, varClass, name);
                        uniforms.put(name, uniform);
                    }
                }
            }
            br.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static Ubo[] parseProgramUbos(Graphics gfx, String vertexSrc, String fragmentSrc) {
        ArrayList<Ubo> uboList = new ArrayList<>();
        uboList.addAll(parseShaderUbos(gfx, GL_VERTEX_SHADER, vertexSrc));
        uboList.addAll(parseShaderUbos(gfx, GL_FRAGMENT_SHADER, fragmentSrc));
        HashSet<String> uboNameSet = new HashSet<>();
        for (int i = 0; i < uboList.size(); i++) {
            if (uboNameSet.contains(uboList.get(i).getName())) {
                uboList.remove(i);
                i--;
            } else {
                uboNameSet.add(uboList.get(i).getName());
            }
        }
        Ubo[] ubos = new Ubo[uboList.size()];
        for (int i = 0; i < ubos.length; i++) ubos[i] = uboList.get(i);
        return ubos;
    }

    private static List<Ubo> parseShaderUbos(Graphics gfx, int type, String src) {
        ArrayList<Ubo> uboList = new ArrayList<>();
        try {
            BufferedReader br = new BufferedReader(new StringReader(src));
            StringBuilder str = null;
            int bindingIndex = -1;
            String name = null;
            String line;
            Map<String, String> constants = new HashMap<>();
            while ((line = br.readLine()) != null) {
                line = line.trim();
                if (line.startsWith("const")) {
                    String[] split = line.split(" ");
                    String constName = split[2];
                    String valueStr = line.substring(line.indexOf("=") + 1, line.indexOf(";")).trim();
                    constants.put(constName, valueStr);
                } else if (line.startsWith("layout(")) {
                    int bracketClose = findCloseIndex(line, "layout(".length() - 1, ')');
                    String[] layoutSplit = line.substring("layout(".length(), bracketClose).split(",");
                    for (String arg : layoutSplit) {
                        arg = arg.trim();
                        String[] split = arg.split("=");
                        String argName = split[0];
                        String value = null;
                        if (split.length > 1) value = split[1];

                        if (argName.equals("std140")) {
                            str = new StringBuilder();
                        } else if (argName.equals("binding")) {
                            bindingIndex = Integer.parseInt(value);
                        }
                    }
                    int uniformKeywordStart = line.indexOf("uniform");
                    bracketClose = line.indexOf("{");
                    if (bracketClose == -1) bracketClose = line.length();
                    name = line.substring(uniformKeywordStart + "uniform".length(), bracketClose).trim();
                }
                if (str != null) {
                    str.append(line);
                }
                if (str != null && line.contains("}")) {
                    if (bindingIndex == -3) throw new GLSLCompileException(type, "must specify ubo binding in glsl");
                    if (name == null) throw new GLSLCompileException(type, "no ubo name found");
                    int bracketOpen = str.indexOf("{");
                    int bracketClose = str.indexOf("}");
                    String block = str.substring(bracketOpen + 1, bracketClose);
                    String[] assignments = block.split(";");
                    for (int i = 0; i < assignments.length; i++) {
                        for (Map.Entry<String, String> entry : constants.entrySet()) {
                            assignments[i] = assignments[i].replace(entry.getKey(), entry.getValue());
                        }
                    }

                    uboList.add(new Ubo(gfx, name, bindingIndex, assignments));
                    str = null;
                    bindingIndex = -3;
                    name = null;
                }
            }
            br.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return uboList;
    }

    private static int findCloseIndex(String str, int bracketOpen, char close) {
        char open = str.charAt(bracketOpen);
        int stack = 1;
        for (int i = bracketOpen + 1; i < str.length(); i++) {
            if (str.charAt(i) == open) stack++;
            if (str.charAt(i) == close) {
                stack--;
                if (stack == 0) return i;
            }
        }
        return -1;
    }

    protected void setBoundStatus(boolean bound) {
        this.bound = bound;
    }

    public boolean isBound() {
        return bound;
    }
}
