package org.aspect.graphics.drawable.attachments;

import org.aspect.graphics.drawable.Attachment;
import org.aspect.graphics.renderer.CullFace;

import static org.lwjgl.opengl.GL11.*;

public class CullFaceAttachment implements Attachment {

    private CullFace cullFace;

    public CullFaceAttachment(CullFace cullFace) {
        this.cullFace = cullFace;
    }

    @Override
    public void bind() {
        if (cullFace.isCulling()) {
            glEnable(GL_CULL_FACE);
            glCullFace(cullFace.getFaceGlID());
        } else {
            glDisable(GL_CULL_FACE);
        }
    }

    public void unbind() {
        glDisable(GL_CULL_FACE);
    }
}
