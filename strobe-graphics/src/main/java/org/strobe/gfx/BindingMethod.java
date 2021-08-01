package org.strobe.gfx;

public interface BindingMethod<T extends Bindable> {
    void bind(Graphics gfx, T bindable);
    void unbind(Graphics gfx, T bindable);
}
