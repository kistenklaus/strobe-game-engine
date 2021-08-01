package org.strobe.gfx.opengl.bindables.shader;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.DefaultPool;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;

import java.util.HashMap;
import java.util.Map;

@Pool(DefaultPool.class)
public final class ShaderUniformSet extends Bindable<DefaultPool> {

    private final Map<String, UniformAssignment> assignments = new HashMap<>();

    ShaderUniformSet(Graphics gfx, Map<String, ShaderUniform> uniforms) {
        super(gfx);
        for (Map.Entry<String, ShaderUniform> entry : uniforms.entrySet()) {
            String name = entry.getKey();
            ShaderUniform uniform = entry.getValue();
            assignments.put(name, new UniformAssignment(uniform, null));
        }
    }

    @Override
    protected void bind(Graphics gfx) {
        for (UniformAssignment assignment : assignments.values()) {
            assignment.assign(gfx);
        }
    }

    public <T> void set(String name, T value) {
        UniformAssignment uniform = assignments.get(name);
        if(uniform == null) throw new IllegalArgumentException("uniform set doesn't contain a uniform ["+name+"]");
        if (!uniform.getType().equals(value.getClass()))
            throw new IllegalArgumentException(name + " is not of type :" + value.getClass());
        ((UniformAssignment<T>) uniform).set(value);
    }

    @Override
    protected void unbind(Graphics gfx) {
        //do nothing
    }

    @Override
    protected void dispose(Graphics gfx) {
        //nothing to dispose just lose the refrence to this object then all assignments get gcc
    }

    private static class UniformAssignment<T> {
        private final ShaderUniform<T> uniform;
        private T assignment;

        public UniformAssignment(ShaderUniform<T> uniform, T assignment) {
            this.uniform = uniform;
            this.assignment = assignment;
        }

        public void set(T assignment) {
            this.assignment = assignment;
        }

        public Class<T> getType() {
            return uniform.getType();
        }

        public void assign(Graphics gfx) {
            if (assignment == null) return;
            uniform.set(gfx, assignment);
        }
    }
}
