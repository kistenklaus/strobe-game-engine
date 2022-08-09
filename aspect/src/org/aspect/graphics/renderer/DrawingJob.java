package org.aspect.graphics.renderer;

import org.aspect.graphics.renderer.drawable.Drawable;
import org.aspect.graphics.bindable.shader.ShaderBindable;
import org.aspect.graphics.renderer.drawable.Step;

public class DrawingJob {

    private final Drawable drawable;
    private final Step step;
    private final Renderer renderer;

    public DrawingJob(Drawable drawable, Step step, Renderer renderer) {
        this.drawable = drawable;
        this.step = step;
        this.renderer = renderer;
    }

    public void execute() {

        drawable.getVertexArray().bind();
        renderer.setDrawCount(drawable.getVertexArray().getDrawCount());

        step.getShader().bind();

        for (ShaderBindable bindable : step.getBindables()) {
            bindable.bind();
            bindable.prepare(renderer, step.getShader());
        }

        renderer.executeDrawCall();

        for (org.aspect.graphics.bindable.Bindable bindable : step.getBindables()) {
            bindable.unbind();
        }

        step.getShader().unbind();

        drawable.getVertexArray().unbind();
    }
}
