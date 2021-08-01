package org.strobe.gfx;

public final class UnconditionalBinding implements BindingMethod<Bindable>{

    public void bind(Graphics gfx, Bindable bindable) {
        bindable.bind(gfx);
    }

    public void unbind(Graphics gfx, Bindable bindable) {
        //do nothing
    }
}
