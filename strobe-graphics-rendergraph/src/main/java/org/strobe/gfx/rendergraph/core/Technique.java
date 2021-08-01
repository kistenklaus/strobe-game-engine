package org.strobe.gfx.rendergraph.core;

import java.util.Arrays;

public final class Technique {

    private boolean enabled = true;

    private Step[] steps;

    public Technique(Step...steps){
        this.steps = steps;
    }

    public void enable(){
        this.enabled = true;
    }

    public void disable(){
        this.enabled = false;
    }

    public boolean isEnabled(){
        return enabled;
    }

    public void setSteps(Step...steps){
        this.steps = steps;
    }

    public Step[] steps(){
        return steps;
    }

    @Override
    public String toString() {
        return "Technique{" +
                "enabled=" + enabled +
                ", steps=" + Arrays.toString(steps) +
                '}';
    }
}
