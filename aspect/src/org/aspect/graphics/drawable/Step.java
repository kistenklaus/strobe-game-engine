package org.aspect.graphics.drawable;

import org.aspect.exceptions.AspectStepLinkingException;
import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.drawable.intern.DrawingJob;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.RenderGraph;
import org.aspect.graphics.passes.Pass;
import org.aspect.graphics.passes.queues.RenderQueuePass;

import java.util.ArrayList;
import java.util.List;

public class Step {

    private final String passName;
    private final Shader shader;
    private RenderQueuePass queue;
    private boolean isLinked;

    private List<Attachment> attachments = new ArrayList<>();

    public Step(String passName, Shader shader) {
        this.passName = passName;
        this.shader = shader;
        this.isLinked = false;
    }

    public synchronized void addAttachment(Attachment attachment) {
        attachments.add(attachment);
    }

    public void link(RenderQueuePass queue) {
        this.queue = queue;
        this.isLinked = true;
    }

    public synchronized void submit(Renderer renderer, RenderGraph graph, Drawable drawable) {
        if (!isLinked) {
            Pass pass = graph.getPass(passName);
            if(pass == null)
                throw new AspectStepLinkingException("the pass the step is referencing is not exiting");
            if (pass instanceof RenderQueuePass)
                link((RenderQueuePass) pass);
            else throw new AspectStepLinkingException("the pass the step is referencing is not a render queue");
        }
        graph.addJob(new DrawingJob(drawable, this, renderer), queue);
    }

    public List<Attachment> getAttachments() {
        return attachments;
    }

    public Shader getShader() {
        return shader;
    }
}
