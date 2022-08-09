package org.aspect.graphics.drawable.techniques;

import org.aspect.graphics.data.Material;
import org.aspect.graphics.drawable.Technique;
import org.aspect.graphics.drawable.steps.LambertianMaterialStep;
import org.aspect.graphics.passes.queues.RenderQueuePass;

public class LambertianMaterialTechnique extends Technique {

    public LambertianMaterialTechnique(String forwardQueueName, Material material){
        addStep(new LambertianMaterialStep(forwardQueueName,material));
    }

    public LambertianMaterialTechnique(RenderQueuePass forwardQueue, Material material){
        this(forwardQueue.getName(), material);
    }
}
