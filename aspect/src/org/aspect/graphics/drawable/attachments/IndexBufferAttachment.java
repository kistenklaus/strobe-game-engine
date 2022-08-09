package org.aspect.graphics.drawable.attachments;

import org.aspect.graphics.bindable.IndexBuffer;
import org.aspect.graphics.renderer.Renderer;

public class IndexBufferAttachment extends BindableAttachment<IndexBuffer> implements RenderAttachment {

    public IndexBufferAttachment(IndexBuffer ibo) {
        super(ibo);
    }

    @Override
    public void prepareRenderer(Renderer renderer) {
        renderer.setDrawCount(getBindable().getSize());
    }
}
