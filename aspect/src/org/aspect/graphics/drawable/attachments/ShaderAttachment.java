package org.aspect.graphics.drawable.attachments;

import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.drawable.Attachment;

public interface ShaderAttachment extends Attachment {

    void prepareShader(Shader shader);
}
