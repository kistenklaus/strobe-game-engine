package org.strobe.gfx;

import java.util.ArrayList;

public abstract class BindablePool <T extends Bindable> implements Iterable<T>{

    protected BindingMethod method;

    public BindablePool(BindingMethod method){
        this.method = method;
    }

    public void bind(Graphics gfx, T bindable){
        method.bind(gfx, bindable);
    }

    public void unbind(Graphics gfx, T bindable){
        method.unbind(gfx, bindable);
    }

    public void dispose(Graphics gfx){
        ArrayList<T> bindables = new ArrayList<>();
        for(T bindable : this) {
            bindables.add(bindable);
        }
        for(int i=bindables.size()-1;i>=0;i--){
            bindables.get(i).dispose(gfx);
        }
    }

    public void dispose(Graphics gfx, T bindable){
        bindable.dispose(gfx);
    }

}
