package org.strobe.gfx;

public final class UnconditionalUnbinding implements BindingMethod<Bindable> {

    @Override
    public void bind(Graphics gfx, Bindable bindable) {
        bindable.bind(gfx);
    }

    @Override
    public void unbind(Graphics gfx, Bindable bindable) {
        bindable.unbind(gfx);
    }
}
