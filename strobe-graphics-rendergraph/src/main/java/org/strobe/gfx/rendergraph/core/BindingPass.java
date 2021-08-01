package org.strobe.gfx.rendergraph.core;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;

import java.util.ArrayList;
import java.util.List;

public abstract class BindingPass extends RenderPass{

    private final List<Bindable> bindables = new ArrayList<>();

    public BindingPass() {
    }


    @Override
    public final void render(Graphics gfx) {
        for(Bindable bindable : bindables)gfx.bind(bindable);
        renderBoundBindables(gfx);
        for(Bindable bindable : bindables)gfx.unbind(bindable);
    }



    @Override
    protected void reset(Graphics gfx) {
        bindables.clear();
        resetImpl(gfx);
    }

    protected void resetImpl(Graphics gfx){}

    public abstract void renderBoundBindables(Graphics gfx);


    public void addBindable(Bindable bindable){
        bindables.add(bindable);
    }
}
