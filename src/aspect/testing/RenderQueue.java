package aspect.testing;

import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.renderer.graph.Sink;
import org.aspect.graphics.renderer.graph.Source;
import org.aspect.graphics.renderer.graph.passes.RenderQueuePass;

public class RenderQueue extends RenderQueuePass {

    private Sink<Framebuffer> target;

    public RenderQueue(String name) {
        super(name);
        registerSink(target=new Sink(Framebuffer.class, "target", "target"));
        registerSource(new Source(Framebuffer.class, "target"));
    }

    @Override
    public void complete() {
        addBindable(target.get());
    }
}
