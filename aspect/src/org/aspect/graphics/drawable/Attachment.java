package org.aspect.graphics.drawable;

import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.renderer.Renderer;

public interface Attachment {

    default void bind() {
    }

    default void prepareShader(Shader shader) {
    }

    default void prepareRenderer(Renderer renderer) {
    }

    default void unbind() {
    }

    default void attach(Shader shader, Renderer renderer){
        prepareShader(shader);
        prepareRenderer(renderer);
        bind();
    }

    default void detach(){
        unbind();
    }
}
