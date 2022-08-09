package org.aspect.graphics.renderer;

import org.aspect.graphics.Graphics;

import java.util.function.Function;

public enum DrawMode {
    TRIANGLE(g -> g.getTriangleMode()),
    LINES(g->g.getLineMode());

    private final Function<Graphics, Integer> apiMapping;

    DrawMode(Function<Graphics, Integer> apiMapping) {
        this.apiMapping = apiMapping;
    }

    public int getAPIMode(Graphics graphics) {
        return apiMapping.apply(graphics);
    }

}
