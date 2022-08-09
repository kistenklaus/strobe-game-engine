package org.aspect.graphics.bindable;

public abstract class VertexBuffer extends Bindable {
    public abstract void create(float[] content, int allocated_size, boolean readonly);
}
