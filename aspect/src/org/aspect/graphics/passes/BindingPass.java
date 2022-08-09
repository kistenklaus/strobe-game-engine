package org.aspect.graphics.passes;


import org.aspect.graphics.bindable.Bindable;

import java.util.ArrayList;

public abstract class BindingPass extends Pass {

    private ArrayList<Bindable> bindables = new ArrayList<>();

    public BindingPass(String name) {
        super(name);
    }

    /**
     * abstract override to keep automatic implementation order
     */
    public abstract void setupSinksAndSources();

    /**
     * abstract override to keep automatic implementation order
     */
    public final void complete(){
        completeBindable();
        completeSettings();
    }

    public abstract void completeBindable();
    public void completeSettings(){}

    @Override
    public final synchronized void execute() {
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
