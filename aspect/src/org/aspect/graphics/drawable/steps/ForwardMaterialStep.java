package org.aspect.graphics.drawable.steps;

import org.aspect.graphics.drawable.Step;
import org.aspect.graphics.drawable.attachments.CullFaceAttachment;
import org.aspect.graphics.material.ForwardMaterial;

public class ForwardMaterialStep extends Step {

    public ForwardMaterialStep(String passName, ForwardMaterial material) {
        super(passName, material.getFMA().getShader());
        addAttachment(material.getFMA());
        addAttachment(new CullFaceAttachment(material.getFMA().getCullFace()));
    }
}
