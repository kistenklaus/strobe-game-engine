package org.aspect.graphics.drawable.attachments;

import org.aspect.graphics.drawable.Attachment;
import org.aspect.graphics.renderer.Renderer;

public interface RenderAttachment extends Attachment {
    void prepareRenderer(Renderer renderer);
}
