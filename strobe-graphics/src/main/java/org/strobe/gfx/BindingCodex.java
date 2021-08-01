package org.strobe.gfx;

import org.strobe.debug.Debug;
import org.strobe.debug.Debuggable;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public final class BindingCodex implements Debuggable {

    @Debug
    private List<BindablePool<?>> poolList = new ArrayList<>();
    private HashMap<Class<? extends BindablePool>, Integer> poolIndices = new HashMap<>();

    public BindingCodex() {

    }

    public <P extends BindablePool> P getPool(Class<P> poolClass) {
        Integer poolIndex = poolIndices.get(poolClass);
        if (poolIndex != null) return (P) poolList.get(poolIndex);

        try {
            P pool = poolClass.getConstructor().newInstance();
            int index = poolList.size();
            poolList.add(pool);
            poolIndices.put(poolClass, index);
            return pool;
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
        throw new IllegalStateException();
    }

    public void dispose(Graphics gfx) {
        for (BindablePool pool : poolList) {
            pool.dispose(gfx);
        }
    }

}
