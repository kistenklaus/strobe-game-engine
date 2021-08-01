package org.strobe.gfx.opengl.bindables.ubo;

import org.strobe.debug.Debug;
import org.strobe.debug.Debuggable;
import org.strobe.gfx.BindablePool;

import java.util.*;

public class UboPool extends BindablePool<Ubo> implements Debuggable {

    @Debug
    private final List<Ubo> ubos = new ArrayList<>();
    private final Map<String, Integer> indices = new HashMap<>();

    public UboPool() {
        super(new UboBindingMethod());
    }

    public void addUbo(Ubo ubo, String... layout) {
        String hash = hashUbo(ubo.getName(), ubo.getBindingIndex(), layout);
        if(indices.containsKey(hash))return;
        int index = ubos.size();
        ubos.add(ubo);
        indices.put(hash, index);
    }

    private String hashUbo(String name, int bindingIndex, String... layout) {
        return "[name=" + name + ",bindingIndex=" + bindingIndex + ",layout=" + Arrays.toString(layout) + "]";
    }

    public Ubo registerUbo(String name, int bindingIndex, String... layout) {
        String hash = hashUbo(name, bindingIndex, layout);
        Integer index = indices.get(hash);
        if(index != null)return ubos.get(index);
        return null;
    }

    @Override
    public Iterator<Ubo> iterator() {
        return ubos.iterator();
    }
}
