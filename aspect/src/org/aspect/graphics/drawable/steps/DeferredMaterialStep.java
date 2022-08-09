package org.aspect.graphics.drawable.steps;

import org.aspect.graphics.drawable.Step;
import org.aspect.graphics.drawable.attachments.CullFaceAttachment;
import org.aspect.graphics.material.DeferredMaterial;

public class DeferredMaterialStep extends Step {

    public DeferredMaterialStep(String passName, DeferredMaterial material) {
        super(passName, material.getDMA().getShader());
        addAttachment(material.getDMA());
        addAttachment(new CullFaceAttachment(material.getDMA().getCullFace()));
    }
}
