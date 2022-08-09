package org.aspect.graphics.passes.sources;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.bindable.Texture;
import org.aspect.graphics.passes.BindingPass;

import static org.lwjgl.opengl.GL11.*;

public class FramebufferSourcePass extends BindingPass {

    public static final String FBO = "fbo";
    public static final String TEX = "tex";

    private final Framebuffer buffer;
    private final Texture color;

    public FramebufferSourcePass(String name, int width, int height, boolean depthTexture) {
        super(name);

        buffer = Aspect.createFramebuffer(width, height);

        color = Aspect.createTexture(null,
                width,height, 1, false,
                Texture.Format.RGBA, Texture.Type.UNSIGNED_BYTE);
        buffer.attachColorAttachment(0, color, true);
        if(depthTexture){
            buffer.attachDepthTexture();
        }else{
            buffer.attachDepth24Stencil8Attachment(1);
        }
        buffer.complete();

    }

    @Override
    public void setupSinksAndSources() {
        registerSource(Framebuffer.class, FBO, buffer);
    }

    @Override
    public void completeBindable() {
        addBindable(buffer);
    }

    @Override
    public void _execute() {
        glClearColor(0,0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}
