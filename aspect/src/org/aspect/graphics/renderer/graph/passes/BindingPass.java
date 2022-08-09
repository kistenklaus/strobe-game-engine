package org.aspect.graphics.renderer.graph.passes;

import org.aspect.graphics.bindable.Bindable;

import java.util.ArrayList;

public abstract class BindingPass extends Pass {

    private ArrayList<Bindable> bindables = new ArrayList<>();

    public BindingPass(String name) {
        super(name);
    }

    @Override
    public synchronized void execute() {
        for (Bindable bindable : bindables)
            bindable.bind();
        _execute();
        for (Bindable bindable : bindables)
            bindable.unbind();
    }

    public abstract void _execute();

    public synchronized void addBindable(Bindable bindable) {
        bindables.add(bindable);
    }
}
