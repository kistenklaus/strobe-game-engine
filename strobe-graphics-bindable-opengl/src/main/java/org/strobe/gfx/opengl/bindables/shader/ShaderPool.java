package org.strobe.gfx.opengl.bindables.shader;

import org.strobe.debug.Debug;
import org.strobe.debug.Debuggable;
import org.strobe.gfx.BindablePool;
import org.strobe.gfx.BindingMethod;
import org.strobe.gfx.Graphics;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

public class ShaderPool extends BindablePool<Shader> implements Debuggable {

    @Debug
    private final List<Shader> shaders = new ArrayList<>();
    private final HashMap<String, Integer> indices = new HashMap<>();

    public ShaderPool() {
        super(new ShaderBinding());
    }

    public void addShader(String vsSrc, String fsSrc, Shader shader){
        String hash = hashShader(vsSrc, fsSrc);
        Integer index = indices.get(hash);
        if(index != null)return;
        index = shaders.size();
        shaders.add(shader);
        indices.put(hash, index);
    }

    private String hashShader(String vsSrc, String fsSrc){
        return vsSrc + "\n&\n" + fsSrc;
    }

    public Shader getShader(String vsSrc, String fsSrc){
        Integer index = indices.get(hashShader(vsSrc, fsSrc));
        if(index == null)return null;
        return shaders.get(index);
    }

    @Override
    public void dispose(Graphics gfx, Shader shader) {
        shaders.remove(shader);
        shader.dispose(gfx);
    }

    @Override
    public Iterator<Shader> iterator() {
        return shaders.iterator();
    }


    private static class ShaderBinding implements BindingMethod<Shader> {

        private Shader bound = null;

        @Override
        public void bind(Graphics gfx, Shader shader) {
            if(shader == null)throw new IllegalArgumentException();
            if(shader == bound)return;
            if(bound != null)bound.setBoundStatus(false);
            shader.bind(gfx);
            shader.setBoundStatus(true);
            bound = shader;
        }

        @Override
        public void unbind(Graphics gfx, Shader shader) {
            if(shader == null)throw new IllegalArgumentException();
            if(bound != shader)return;
            bound = null;
            shader.setBoundStatus(false);
        }
    }
}
