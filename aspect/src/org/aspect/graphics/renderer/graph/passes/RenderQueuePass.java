package org.aspect.graphics.renderer.graph.passes;

import org.aspect.graphics.renderer.DrawingJob;

import java.util.ArrayList;
import java.util.List;

public abstract class RenderQueuePass extends BindingPass {

    private final List<DrawingJob> jobs = new ArrayList<>();

    public RenderQueuePass(String name) {
        super(name);
    }


    public synchronized void addJob(DrawingJob job) {
        jobs.add(job);
    }


    public synchronized void _execute() { ;
        for (DrawingJob job : jobs) {
            job.execute();
        }
    }

    public synchronized void reset() {
        jobs.clear();
    }
}
