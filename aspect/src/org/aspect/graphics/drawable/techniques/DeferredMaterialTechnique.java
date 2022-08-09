package org.aspect.graphics.drawable.techniques;

import org.aspect.graphics.data.Material;
import org.aspect.graphics.drawable.Technique;
import org.aspect.graphics.drawable.steps.DeferredMaterialStep;
import org.aspect.graphics.passes.queues.RenderQueuePass;

public class DeferredMaterialTechnique extends Technique {

    public DeferredMaterialTechnique(String deferredQueueName, Material material){
        addStep(new DeferredMaterialStep(deferredQueueName,material));
    }

    public DeferredMaterialTechnique(RenderQueuePass deferredQueue, Material material){
        this(deferredQueue.getName(), material);
    }
}
