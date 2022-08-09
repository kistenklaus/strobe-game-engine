package org.aspect.graphics.passes.sources;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.bindable.Texture;
import org.aspect.graphics.passes.BindingPass;

import static org.lwjgl.opengl.GL11.*;

public class GBufferSourcePass extends BindingPass {

    public static final String GBUFFER = "gbuffer";

    public static final int position_location = 0;

    public static final int normal_location = 1;
    public static final int albedoSpec_location = 2;

    private final Framebuffer gbuffer;
    private final Texture positions, normals, albedoSpec;

    public GBufferSourcePass(String name, int width, int height) {
        super(name);
        gbuffer = Aspect.createFramebuffer(width, height);


        positions = Aspect.createTexture(null,
                width,height, 1, false,
                Texture.Format.RGBA16F, Texture.Type.FLOAT);
        gbuffer.attachColorAttachment(0, positions, true);


        normals = Aspect.createTexture(null,
                width,height,1,false,
                Texture.Format.RGBA16F, Texture.Type.FLOAT);
        gbuffer.attachColorAttachment(1, normals, true);

        albedoSpec = Aspect.createTexture(null,
                width, height,1,false,
                Texture.Format.RGBA, Texture.Type.UNSIGNED_BYTE);
        gbuffer.attachColorAttachment(2, albedoSpec, true);

        gbuffer.attachDepth24Stencil8Attachment(1);

        gbuffer.complete();

    }

    @Override
    public void setupSinksAndSources() {
        registerSource(Framebuffer.class, GBUFFER, gbuffer);
    }

    @Override
    public void completeBindable() {
        addBindable(gbuffer);
    }

    @Override
    public void _execute() {
        glClearColor(0,0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}
