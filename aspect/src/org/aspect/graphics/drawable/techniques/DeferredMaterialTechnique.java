package org.aspect.graphics.drawable.techniques;

import org.aspect.graphics.drawable.Technique;
import org.aspect.graphics.drawable.steps.DeferredMaterialStep;
import org.aspect.graphics.material.DeferredMaterial;

public class DeferredMaterialTechnique extends Technique {

    public DeferredMaterialTechnique(String deferredQueueName, DeferredMaterial material) {
        addStep(new DeferredMaterialStep(deferredQueueName, material));
    }

}
