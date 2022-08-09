package org.aspect.graphics.bindable.shader;

import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.renderer.Renderer;

public abstract class ShaderBindable extends org.aspect.graphics.bindable.Bindable {

    @Override
    public abstract void bind();

    public abstract void prepare(Renderer renderer, Shader shader);

    @Override
    public abstract void unbind();
}
