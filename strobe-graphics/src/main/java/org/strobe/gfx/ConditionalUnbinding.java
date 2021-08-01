package org.strobe.gfx;

public final class ConditionalUnbinding implements BindingMethod<Bindable>{

    private Bindable prev = null;

    @Override
    public void bind(Graphics gfx, Bindable bindable) {
        if(prev != null){
            if(prev.equals(bindable))return;
            prev.unbind(gfx);
        }
        bindable.bind(gfx);
        prev = bindable;
    }

    @Override
    public void unbind(Graphics gfx, Bindable bindable) {
        if(prev == null)return;
        if(!prev.equals(bindable))return;
        prev.unbind(gfx);
        prev = null;
    }
}
