package org.aspect.graphics.drawable.techniques;

import org.aspect.graphics.drawable.Technique;
import org.aspect.graphics.drawable.steps.ForwardMaterialStep;
import org.aspect.graphics.material.ForwardMaterial;

public class ForwardMaterialTechnique extends Technique {

    public ForwardMaterialTechnique(String forwardQueueName, ForwardMaterial material) {
        addStep(new ForwardMaterialStep(forwardQueueName, material));

    }
}
