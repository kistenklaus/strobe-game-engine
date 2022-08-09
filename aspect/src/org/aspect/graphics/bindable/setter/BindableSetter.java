package org.aspect.graphics.bindable.setter;

import org.aspect.graphics.bindable.Bindable;

public abstract class BindableSetter extends Bindable {


    @Override
    public final void create() {
        //--
    }

    @Override
    public final void bind() {
        enable();
    }

    public abstract void enable();

    @Override
    public final void unbind() {
        disable();
    }

    public abstract void disable();

    @Override
    public final void dispose() {
        //--
    }

    @Override
    public final String hash() {
        return null;
    }
}
