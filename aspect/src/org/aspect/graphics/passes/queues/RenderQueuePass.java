package org.aspect.graphics.passes.queues;

import org.aspect.graphics.passes.BindingPass;
import org.aspect.graphics.drawable.intern.DrawingJob;

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

    public synchronized void _execute() {
        prepare();
        for (DrawingJob job : jobs) {
            job.execute();
        }
    }

    public abstract void prepare();

    public synchronized void reset() {
        jobs.clear();
    }


}
