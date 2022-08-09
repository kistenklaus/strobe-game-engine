package org.aspect.graphics.renderer.graph;

import org.aspect.graphics.bindable.Bindable;
import org.aspect.graphics.bindable.UniformBuffer;

public class Source<T> {

    private final String name;
    private String passName;
    private T value = null;
    private Class<T> valueClass;

    public Source(Class<T> valueClass, String name) {
        this.name = name;
        this.valueClass = valueClass;
    }

    public Source(Class<T> valueClass, String name, T value) {
        this.name = name;
        this.valueClass = valueClass;
        this.value = value;
    }


    public String getName() {
        return name;
    }

    public T get() {
        return value;
    }

    public Class<T> getValueClass() {
        return valueClass;
    }

    public void connect(T value) {
        this.value = value;
    }


    public void registerTo(String passName) {
        this.passName = passName;
    }

    public String getPassName() {
        return passName;
    }

    public String toString() {
        return "Source:[" + passName + "." + name + ":" + value + "]";
    }


}
