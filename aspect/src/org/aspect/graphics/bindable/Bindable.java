package org.aspect.graphics.bindable;

public abstract class Bindable {

    public Bindable() {

    }

    public abstract void create();

    public abstract void bind();

    public abstract void unbind();

    public abstract void dispose();

    public abstract String hash();
}
