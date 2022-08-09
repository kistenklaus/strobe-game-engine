package org.aspect.graphics.drawable;

import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.RenderGraph;

import java.util.ArrayList;

public class Technique {

    private final ArrayList<Step> steps = new ArrayList<>();

    public Technique(Step... steps) {
        this.steps.ensureCapacity(steps.length);
        for (int i = 0; i < steps.length; i++)
            this.steps.add(steps[i]);
    }

    public void submit(Renderer renderer, RenderGraph graph, Drawable drawable) {
        for (Step step : steps) {
            step.submit(renderer, graph, drawable);
        }
    }

    public void addStep(Step step) {
        steps.add(step);
    }

    public ArrayList<Step> getSteps() {
        return steps;
    }
}
