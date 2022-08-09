package org.aspect.graphics.renderer.drawable;

import org.aspect.exceptions.AspectStepLinkingException;
import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.bindable.shader.ShaderBindable;
import org.aspect.graphics.renderer.DrawingJob;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.RenderGraph;
import org.aspect.graphics.renderer.graph.passes.Pass;
import org.aspect.graphics.renderer.graph.passes.RenderQueuePass;

import java.util.ArrayList;
import java.util.List;
import java.util.function.BiConsumer;
import java.util.function.Consumer;

public class Step {

    private final String passName;
    private final Shader shader;
    private RenderQueuePass queue;
    private boolean isLinked;
    private List<ShaderBindable> bindables = new ArrayList<>();

    public Step(String passName, Shader shader) {
        this.passName = passName;
        this.shader = shader;
        this.isLinked = false;
    }

    public Step(RenderQueuePass queue, Shader shader) {
        this.passName = queue.getName();
        this.queue = queue;
        this.shader = shader;
        this.isLinked = true;
    }

    public synchronized void addBindable(ShaderBindable bindable) {
        this.bindables.add(bindable);
    }

    public synchronized void addPreparer(BiConsumer<Renderer, Shader> preparer) {
        addBindable(new ShaderBindable() {
            @Override
            public void bind() {
            }

            @Override
            public void prepare(Renderer renderer, Shader shader) {
                preparer.accept(renderer, shader);
            }

            @Override
            public void unbind() {
            }

            @Override
            public void dispose() {
            }
        });
    }

    public synchronized void addSetting(Consumer<Renderer> setter) {
        addPreparer((r,s)->setter.accept(r));
    }

    public synchronized void addShaderTask(Consumer<Shader> task) {
        addPreparer((r,s)->task.accept(s));
    }

    public void link(RenderQueuePass queue) {
        this.queue = queue;
        this.isLinked = true;
    }

    public synchronized void submit(RenderGraph graph, Drawable drawable) {
        //checking on every submission is bad (because submission is every frame!)
        if (!isLinked) {
            Pass pass = graph.getPass(passName);
            if (pass instanceof RenderQueuePass)
                link((RenderQueuePass) pass);
            else throw new AspectStepLinkingException("the pass the step is referencing is not a render queue");
        }
        graph.addJob(new DrawingJob(drawable, this, graph.getRenderer()), queue);
    }

    public List<ShaderBindable> getBindables() {
        return bindables;
    }

    public Shader getShader() {
        return shader;
    }

    public String getPassName() {
        return passName;
    }

    public boolean isLinked() {
        return isLinked;
    }
}
