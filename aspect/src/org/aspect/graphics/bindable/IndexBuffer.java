package org.aspect.graphics.bindable;

public abstract class IndexBuffer extends Bindable {
    public abstract void create(int[] content, int allocated_ints, boolean readonly);

    public abstract int getSize();
}
