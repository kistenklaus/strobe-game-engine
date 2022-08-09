package org.aspect.graphics.bindable.shader;

import org.aspect.graphics.bindable.IndexBuffer;
import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.renderer.Renderer;

public class ShaderIndexBuffer extends ShaderBindable{

    private IndexBuffer ibo;

    public void create(IndexBuffer ibo){
        this.ibo = ibo;
    }

    @Override
    public void bind() {
        ibo.bind();
    }

    @Override
    public void prepare(Renderer renderer, Shader shader) {
        renderer.setDrawCount(ibo.getSize());
    }

    @Override
    public void unbind() {
        ibo.unbind();
    }

    @Override
    public void dispose() {
        ibo.dispose();
    }
}
