package org.strobe.gfx.rendergraph.core;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.materials.shaders.MaterialShader;

import java.util.Arrays;

public final class Step {

    private final MaterialShader shader;
    private final Bindable[] bindables;
    private final RenderQueue renderQueue;

    public Step(RenderQueue renderQueue, MaterialShader shader, Bindable[] bindables) {
        this.shader = shader;
        this.bindables = bindables;
        this.renderQueue = renderQueue;
    }

    protected MaterialShader getShader() {
        return shader;
    }

    protected Bindable[] getBindables() {
        return bindables;
    }

    protected RenderQueue getRenderQueue() {
        return renderQueue;
    }

    @Override
    public String toString() {
        return "Step{" +
                "shader=" + shader +
                ", bindables=" + Arrays.toString(bindables) +
                ", renderQueue=" + renderQueue +
                '}';
    }
}
