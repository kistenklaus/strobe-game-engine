package org.aspect.graphics.passes.utility;

import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.passes.Pass;
import org.aspect.graphics.renderer.BufferBits;
import org.aspect.graphics.renderer.InterpMode;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.Source;

public class BlitBufferPass extends Pass {

    public static final String READ_BUFFER = "read-buffer";
    public static final String DRAW_BUFFER = "draw-buffer";

    private Source<Framebuffer> read;
    private Source<Framebuffer> draw;
    private BufferBits blitTarget;
    private InterpMode interpMode;
    private Renderer renderer;

    public BlitBufferPass(String name, Renderer renderer, InterpMode interpMode, BufferBits blitTarget) {
        super(name);
        this.renderer = renderer;
        this.blitTarget = blitTarget;
        this.interpMode = interpMode;
    }

    public BlitBufferPass(String name, Renderer renderer, BufferBits blitTarget) {
        super(name);
        this.renderer = renderer;
        this.blitTarget = blitTarget;
        this.interpMode = InterpMode.NEAREST;
    }

    @Override
    public void setupSinksAndSources() {
        read = registerSinkSource(Framebuffer.class,READ_BUFFER);
        draw = registerSinkSource(Framebuffer.class,DRAW_BUFFER);
    }

    @Override
    public void complete() {
    }

    @Override
    public void execute() {
        renderer.blitFramebuffer(read.get(), draw.get(), blitTarget, interpMode);
    }
}
