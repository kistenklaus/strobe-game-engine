package org.aspect.graphics.drawable.intern;

import org.aspect.graphics.drawable.Drawable;
import org.aspect.graphics.drawable.Step;
import org.aspect.graphics.drawable.Attachment;
import org.aspect.graphics.drawable.Mesh;
import org.aspect.graphics.renderer.Renderer;

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
        Mesh mesh = drawable.getMesh();
        mesh.getVao().bind();
        renderer.setDrawMethod(mesh.getMethod());
        renderer.setDrawCount(mesh.getDrawCount());
        renderer.setInstanceCount(mesh.getInstanceCount());
        renderer.setDrawMode(mesh.getMode());

        step.getShader().bind();

        //inefficiency due to multiple updates per frame (06.04.2021)
        drawable.getTransform().updateModelUniform(step.getShader());

        for (Attachment attachment : step.getAttachments()) {
            attachment.attach(step.getShader(), renderer);
        }

        renderer.executeDrawCall();

        for (Attachment attachment : step.getAttachments()) {
            attachment.detach();
        }

        step.getShader().unbind();

        mesh.getVao().unbind();
    }
}
