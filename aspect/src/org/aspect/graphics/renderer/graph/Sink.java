package org.aspect.graphics.renderer.graph;

public class Sink<T> {

    private final String name;
    private final String correlatedSource;
    private String passName;
    private T value;
    private Class<T> valueClass;

    public Sink(Class<T> valueClass, String name, String source) {
        this.name = name;
        this.valueClass = valueClass;
        this.correlatedSource = source;
    }

    public void link(T value) {
        this.value = value;
    }

    public T get() {
        return value;
    }

    public String getName() {
        return name;
    }

    public String getCorrelatedSourceName() {
        return correlatedSource;
    }

    public Class<T> getValueClass() {
        return valueClass;
    }

    public void registerTo(String passName) {
        this.passName = passName;
    }

    public String getLocation(){
        return passName+"."+name;
    }

    public String toString() {
        return "Sink:[" + passName + "." + name + ":" + value + "]";
    }


}
