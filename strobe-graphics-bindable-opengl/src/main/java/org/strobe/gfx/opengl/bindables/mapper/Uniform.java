package org.strobe.gfx.opengl.bindables.mapper;

import org.strobe.gfx.Graphics;

public interface Uniform<T> {
    /**
     * sets the value of the uniform
     * @param gfx the graphics
     * @param value the value to the the uniform to
     */
    void set(Graphics gfx, T value);

    /**
     * gets the current value of the uniform
     * @implNote for some uniforms this will not be available.
     * @implNote getting the value of a uniform variable stored in memory can be very slow.
     * @implNote this method should only be used for testing and debugging
     * @param gfx the graphics
     * @return the value of the uniform.
     */
    T get(Graphics gfx);
}
