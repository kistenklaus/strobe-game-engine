package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.renderables.opengl.ScreenVao;
import org.strobe.gfx.rendergraph.core.RenderPass;
import org.strobe.gfx.rendergraph.core.Resource;

import java.util.ArrayList;
import java.util.Arrays;

import static org.lwjgl.opengl.GL11.*;

public final class PostProcessingPass extends RenderPass {

    private static final String TARGET_RESOURCE = "target";
    private static final String SOURCE_FLIP_RESOURCE = "fsource";
    private static final String SOURCE_RESOURCE = "source";

    private final Resource<Framebuffer> target;
    private final Resource<Framebuffer> fsource;
    private final Resource<Framebuffer> source;

    private final ArrayList<PostProcessingFilter> filters = new ArrayList<>();

    private int scaleFilter = GL_NEAREST;

    protected final Renderable screen;

    public PostProcessingPass(Graphics gfx, PostProcessingFilter... filters) {
        target = registerResource(Framebuffer.class, TARGET_RESOURCE);
        source = registerResource(Framebuffer.class, SOURCE_RESOURCE);
        fsource = registerResource(Framebuffer.class, SOURCE_FLIP_RESOURCE);
        screen = new ScreenVao(gfx);
        this.filters.addAll(Arrays.asList(filters));
    }

    public void addFilter(PostProcessingFilter filter) {
        this.filters.add(filter);
    }

    public void removeFilter(PostProcessingFilter filter) {
        this.filters.remove(filter);
    }

    public void resetFilters(){
        filters.clear();
    }

    @Override
    public void complete(Graphics gfx) {
        if (source.get().getWidth() != fsource.get().getWidth()
                || source.get().getHeight() != fsource.get().getHeight())
            throw new IllegalStateException();
    }

    @Override
    protected void reset(Graphics gfx) {

    }

    @Override
    protected void render(Graphics gfx) {
        Framebuffer front = source.get();
        Framebuffer back = fsource.get();
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        for (PostProcessingFilter filter : filters) {
            gfx.bind(back);
            filter.prepareFSR(gfx, front);
            screen.render(gfx);
            gfx.unbind(back);

            Framebuffer temp = front;
            front = back;
            back = temp;
        }
        front.copyTo(gfx, target.get(), GL_COLOR_BUFFER_BIT, scaleFilter);
    }

    @Override
    public void dispose(Graphics gfx) {
        //stub
    }

    public void setScaleFilter(int scaleFilter){
        this.scaleFilter = scaleFilter;
    }

    public Resource<Framebuffer> getTargetResource() {
        return target;
    }

    public Resource<Framebuffer> getSourceResource() {
        return source;
    }

    public Resource<Framebuffer> getSourceFlipResource() {
        return fsource;
    }
}
