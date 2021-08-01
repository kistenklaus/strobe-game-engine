package org.strobe.ecs.context.renderer;

import org.strobe.ecs.*;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.rendergraph.common.*;
import org.strobe.gfx.rendergraph.core.RenderGraphRenderer;
import org.strobe.gfx.rendergraph.core.RenderQueue;
import org.strobe.gfx.rendergraph.core.Resource;

import java.util.Collection;
import java.util.LinkedList;
import java.util.function.BiConsumer;
import java.util.function.Consumer;

import static org.lwjgl.opengl.GL11.GL_COLOR_BUFFER_BIT;
import static org.lwjgl.opengl.GL11.GL_DEPTH_BUFFER_BIT;

public final class EntityRenderer extends RenderGraphRenderer {

    private static final int POINT_LIGHT_COUNT = 10;
    private static final int DIRECTIONAL_LIGHT_COUNT = 3;

    private static final int SHADOW_MAP_WIDTH = 1024;
    private static final int SHADOW_MAP_HEIGHT = 1024;

    private static final String GLOBAL_CAMERA_RESOURCE = "globalCameraResource";
    private static final String GLOBAL_LIGHTS_RESOURCE = "globalLightsResource";
    private static final String GLOBAL_BACK_BUFFER_RESOURCE = "globalBackBufferResource";
    private static final String GLOBAL_TARGET_BUFFER_RESOURCE = "globalTargetBufferResource";
    private static final String GLOBAL_POST_PROCESSING_BUFFER_RESOURCE = "globalPostProcessingBufferResource";
    private static final String GLOBAL_SHADOW_MAP_RESOURCE = "globalShadowMapResource";

    private static final ComponentMapper<MeshRenderer> MESH_RENDERER = ComponentMapper.getFor(MeshRenderer.class);
    private static final ComponentMapper<Mesh> MESH = ComponentMapper.getFor(Mesh.class);
    private static final ComponentMapper<Material> MATERIAL = ComponentMapper.getFor(Material.class);
    private static final ComponentMapper<Transform> TRANSFORM = ComponentMapper.getFor(Transform.class);


    private enum RendererState {
        DISABLED_RENDERER,
        RENDERER_3D,
    }

    private final Framebuffer.Attachment[] targetAttachments = {
            Framebuffer.Attachment.COLOR_RGBA_ATTACHMENT_0, Framebuffer.Attachment.DEPTH_ATTACHMENT
    };

    private RendererState rendererState = RendererState.DISABLED_RENDERER;

    private final ForwardQueue forwardQueue = new ForwardQueue();
    private final ClearFramebufferPass targetClearPass = new ClearFramebufferPass(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    private final Resource<Framebuffer> globalBackBufferResource;


    private final LinkedList<BiConsumer<Graphics, EntityRenderer>> renderOps = new LinkedList<>();

    private final LinkedList<Runnable> sceneOps = new LinkedList<>();

    private PostProcessingFilter fxaaFilter = null;


    public EntityRenderer(Graphics gfx, EntityComponentSystem ecs) {
        ecs.addEntitySystem(new MeshRendererSystem(ecs, this));
        ecs.addEntitySystem(new MaterialSystem(ecs, this));
        ecs.addEntitySystem(new TransformSystem(ecs));


        addPass(forwardQueue);
        addPass(targetClearPass);

        Framebuffer backBuffer = Framebuffer.getBackBuffer(gfx);
        globalBackBufferResource = registerResource(Framebuffer.class, GLOBAL_BACK_BUFFER_RESOURCE, backBuffer);

        addLinkage(globalBackBufferResource, targetClearPass.getTargetResource());
        addLinkage(targetClearPass.getTargetResource(), forwardQueue.getTargetResource());

    }

    @Override
    public void beforeRender(Graphics gfx) {
        while(!renderOps.isEmpty())renderOps.pop().accept(gfx, this);
    }

    public RenderQueue getForwardQueue() {
        return forwardQueue;
    }

    public void enqueueRenderOps(Collection<BiConsumer<Graphics, EntityRenderer>> renderOp){
        renderOps.addAll(renderOp);
    }

    public void enqueueRenderOp(BiConsumer<Graphics, EntityRenderer> renderOp){
        renderOps.add(renderOp);
    }

}
