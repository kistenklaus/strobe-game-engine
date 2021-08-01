package org.strobe.gfx;

public final class ConditionalBinding implements BindingMethod<Bindable>{

    private Bindable prev = null;

    @Override
    public void bind(Graphics gfx, Bindable bindable) {
        if(prev != null && prev.equals(bindable))return;
        bindable.bind(gfx);
        prev = bindable;
    }

    @Override
    public void unbind(Graphics gfx, Bindable bindable) {
        //do nothing
    }
}
