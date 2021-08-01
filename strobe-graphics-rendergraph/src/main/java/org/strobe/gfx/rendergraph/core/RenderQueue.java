package org.strobe.gfx.rendergraph.core;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.materials.shaders.MaterialShader;
import org.strobe.gfx.transform.AbstractTransform;

import java.util.ArrayList;
import java.util.List;

public abstract class RenderQueue extends BindingPass{

    protected final List<RenderQueue.Job> queue = new ArrayList<>();

    public RenderQueue() {
    }

    public final void submit(GraphDrawable drawable, Step step){
        queue.add(new RenderQueue.Job(drawable, step));
    }

    public abstract void renderDrawable(Graphics gfx, AbstractTransform transform, Renderable renderable,
                                        MaterialShader shader, Bindable[] bindables);


    public void beforeRender(Graphics gfx){}

    @Override
    public final void renderBoundBindables(Graphics gfx) {
        beforeRender(gfx);
        renderQueue(gfx);
        queue.clear();
        afterRender(gfx);
    }

    protected void renderQueue(Graphics gfx){
        for(RenderQueue.Job job : queue)job.execute(gfx);
    }

    public void afterRender(Graphics gfx){}

    @Override
    public void dispose(Graphics gfx) {

    }

    protected final class Job{
        protected GraphDrawable drawable;
        protected Step step;

        public Job(GraphDrawable drawable, Step step) {
            this.drawable = drawable;
            this.step = step;
        }

        public void execute(Graphics gfx){
            RenderQueue.this.renderDrawable(gfx, drawable.getTransform(), drawable.getRenderable(), step.getShader(), step.getBindables());
        }
    }
}
