package org.aspect.datatype;

public interface Filter<T> {
    boolean matches(T entry);
}
