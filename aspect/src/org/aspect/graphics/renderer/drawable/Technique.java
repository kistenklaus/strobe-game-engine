package org.aspect.graphics.renderer.drawable;

import org.aspect.graphics.renderer.graph.RenderGraph;

import java.util.ArrayList;

public class Technique {

    private final ArrayList<Step> steps = new ArrayList<>();

    public Technique(Step... steps) {
        this.steps.ensureCapacity(steps.length);
        for (int i = 0; i < steps.length; i++)
            this.steps.add(steps[i]);
    }

    public void submit(RenderGraph graph, Drawable drawable) {
        for (Step step : steps) {
            step.submit(graph, drawable);
        }
    }

    public void addStep(Step step) {
        steps.add(step);
    }

    public ArrayList<Step> getSteps() {
        return steps;
    }
}
