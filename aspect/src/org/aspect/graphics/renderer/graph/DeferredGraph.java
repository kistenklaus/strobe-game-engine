package org.aspect.graphics.renderer.graph;

import org.aspect.graphics.data.Camera;
import org.aspect.graphics.passes.effects.LightPass;
import org.aspect.graphics.passes.effects.PostProcessingPass;
import org.aspect.graphics.passes.queues.SceneQueuePass;
import org.aspect.graphics.passes.sources.CameraSourcePass;
import org.aspect.graphics.passes.sources.FramebufferSourcePass;
import org.aspect.graphics.passes.sources.GBufferSourcePass;
import org.aspect.graphics.passes.sources.LightSourcePass;
import org.aspect.graphics.passes.utility.BlitBufferPass;
import org.aspect.graphics.passes.utility.BufferClearPass;
import org.aspect.graphics.renderer.BufferBits;
import org.aspect.graphics.renderer.Renderer;

public class DeferredGraph extends RenderGraph {

    public static final int RENDER_FORWARD = 0;
    public static final int RENDER_DEFERRED = 1;

    private static final String DEFERRED_QUEUE = "deferred-queue";
    private static final String FORWARD_QUEUE = "forward-queue";
    private static final String POST_PROCESSING = "post-processing";
    private static final String CLEAR_BUFFER = "clear-buffer";
    private static final String FRAMEBUFFER_SOURCE = "fbo-source";
    private static final String GBUFFER_SOURCE = "gbuffer-source";
    private static final String LIGHT_PASS = "light-pass";
    private static final String LIGHT_SOURCE = "light-source";
    private static final String CAMERA_SOURCE = "camera-source";
    private static final String DEPTH_BLIT = "depth-blit";

    private static final String DIFFUSE_MAP_UNIFORM_NAME = "diffuseMap";
    private static final String SPECULAR_MAP_UNIFORM_NAME = "specularMap";

    private final Renderer renderer;
    private final Camera camera;
    private int width, height;


    public DeferredGraph(Renderer renderer, int width, int height) {
        super();
        this.renderer = renderer;
        this.width = width;
        this.height = height;

        camera = new Camera(Camera.CameraMode.PERSPECTIVE, width, height, 0.1f, 1000);
        camera.setPosition(0, 1, 50);
        camera.setFOV((float) Math.toRadians(60));

        /*
        FramebufferSourcePass targetSource = new FramebufferSourcePass(FRAMEBUFFER_SOURCE, width, height, true);
        BufferClearPass clearPass = new BufferClearPass(CLEAR_BUFFER);
        LightPass lightPass = new LightPass(LIGHT_PASS, renderer);
        GBufferSourcePass gbuffer = new GBufferSourcePass(GBUFFER_SOURCE, width, height);
        DeferredQueue deferredQueue = new DeferredQueue(DEFERRED_QUEUE);
        PostProcessingPass ppPass = new PostProcessingPass(POST_PROCESSING, renderer);
        ForwardQueue forwardQueue = new ForwardQueue(FORWARD_QUEUE);
        LightSourcePass lightSource = new LightSourcePass(LIGHT_SOURCE, 1);
        CameraSourcePass cameraSource = new CameraSourcePass(CAMERA_SOURCE, 0, camera);
        BlitBufferPass depthBlit = new BlitBufferPass(DEPTH_BLIT, renderer, BufferBits.DEPTH);
        addAllPasses(targetSource, lightPass, gbuffer, deferredQueue,
                ppPass, clearPass, forwardQueue, lightSource, cameraSource, depthBlit);
        //screen:
        setLinkage("$.back_buffer", clearPass.addressOf(BufferClearPass.TARGET));
        setLinkage(clearPass.addressOf(BufferClearPass.TARGET), ppPass.addressOf(PostProcessingPass.SCREEN));
        setLinkage(ppPass.addressOf(PostProcessingPass.SCREEN), "$.back_buffer");
        //target:
        setLinkage(targetSource.addressOf(FramebufferSourcePass.FBO), lightPass.addressOf(LightPass.TARGET));
        setLinkage(lightPass.addressOf(LightPass.TARGET), depthBlit.addressOf(BlitBufferPass.DRAW_BUFFER));
        setLinkage(depthBlit.addressOf(BlitBufferPass.DRAW_BUFFER), forwardQueue.addressOf(ForwardQueue.TARGET));
        setLinkage(forwardQueue.addressOf(ForwardQueue.TARGET), ppPass.addressOf(PostProcessingPass.TARGET));
        //gbuffer:
        setLinkage(gbuffer.addressOf(GBufferSourcePass.GBUFFER), deferredQueue.addressOf(DeferredQueue.GBUFFER));
        setLinkage(deferredQueue.addressOf(DeferredQueue.GBUFFER), lightPass.addressOf(LightPass.GBUFFER));
        setLinkage(lightPass.addressOf(LightPass.GBUFFER), depthBlit.addressOf(BlitBufferPass.READ_BUFFER));
        setLinkage(depthBlit.addressOf(BlitBufferPass.READ_BUFFER), forwardQueue.addressOf(ForwardQueue.GBUFFER));
        setLinkage(forwardQueue.addressOf(ForwardQueue.GBUFFER), ppPass.addressOf(PostProcessingPass.GBUFFER));
        //lightUbo
        setLinkage(lightSource.addressOf(LightSourcePass.LIGHT_UBO), deferredQueue.addressOf(DeferredQueue.LIGHT_UBO));
        setLinkage(deferredQueue.addressOf(DeferredQueue.LIGHT_UBO), lightPass.addressOf(LightPass.LIGHT_UBO));
        setLinkage(lightPass.addressOf(LightPass.LIGHT_UBO), forwardQueue.addressOf(ForwardQueue.LIGHT_UBO));
        //cameraUbo
        setLinkage(cameraSource.addressOf(CameraSourcePass.CAMERA_UBO), deferredQueue.addressOf(DeferredQueue.CAMERA_UBO));
        setLinkage(deferredQueue.addressOf(DeferredQueue.CAMERA_UBO), lightPass.addressOf(LightPass.CAMERA_UBO));
        setLinkage(lightPass.addressOf(LightPass.CAMERA_UBO), forwardQueue.addressOf(ForwardQueue.CAMERA_UBO));
         */

        FramebufferSourcePass targetSource = new FramebufferSourcePass(FRAMEBUFFER_SOURCE, width, height, true);
        BufferClearPass clearPass = new BufferClearPass(CLEAR_BUFFER);
        LightPass lightPass = new LightPass(LIGHT_PASS, renderer);
        GBufferSourcePass gbuffer = new GBufferSourcePass(GBUFFER_SOURCE, width, height);
        SceneQueuePass deferredQueue = new SceneQueuePass(DEFERRED_QUEUE);
        PostProcessingPass ppPass = new PostProcessingPass(POST_PROCESSING, renderer);
        SceneQueuePass forwardQueue = new SceneQueuePass(FORWARD_QUEUE);
        LightSourcePass lightSource = new LightSourcePass(LIGHT_SOURCE, 1);
        CameraSourcePass cameraSource = new CameraSourcePass(CAMERA_SOURCE, 0, camera);
        BlitBufferPass depthBlit = new BlitBufferPass(DEPTH_BLIT, renderer, BufferBits.DEPTH);
        addAllPasses(targetSource, lightPass, gbuffer, deferredQueue,
                ppPass, clearPass, forwardQueue, lightSource, cameraSource, depthBlit);
        //screen:
        setLinkage("$.back_buffer", clearPass.addressOf(BufferClearPass.TARGET));
        setLinkage(clearPass.addressOf(BufferClearPass.TARGET), ppPass.addressOf(PostProcessingPass.SCREEN));
        setLinkage(ppPass.addressOf(PostProcessingPass.SCREEN), "$.back_buffer");
        //target:
        setLinkage(targetSource.addressOf(FramebufferSourcePass.FBO), lightPass.addressOf(LightPass.TARGET));
        setLinkage(lightPass.addressOf(LightPass.TARGET), depthBlit.addressOf(BlitBufferPass.DRAW_BUFFER));
        setLinkage(depthBlit.addressOf(BlitBufferPass.DRAW_BUFFER), forwardQueue.addressOf(SceneQueuePass.TARGET));
        setLinkage(forwardQueue.addressOf(SceneQueuePass.TARGET), ppPass.addressOf(PostProcessingPass.TARGET));
        //gbuffer:
        setLinkage(gbuffer.addressOf(GBufferSourcePass.GBUFFER), deferredQueue.addressOf(SceneQueuePass.TARGET));
        setLinkage(deferredQueue.addressOf(SceneQueuePass.TARGET), lightPass.addressOf(LightPass.GBUFFER));
        setLinkage(lightPass.addressOf(LightPass.GBUFFER), depthBlit.addressOf(BlitBufferPass.READ_BUFFER));
        setLinkage(depthBlit.addressOf(BlitBufferPass.READ_BUFFER), ppPass.addressOf(PostProcessingPass.GBUFFER));
        //lightUbo
        setLinkage(lightSource.addressOf(LightSourcePass.LIGHT_UBO), deferredQueue.addressOf(SceneQueuePass.LIGHT_UBO));
        setLinkage(deferredQueue.addressOf(SceneQueuePass.LIGHT_UBO), lightPass.addressOf(LightPass.LIGHT_UBO));
        setLinkage(lightPass.addressOf(LightPass.LIGHT_UBO), forwardQueue.addressOf(SceneQueuePass.LIGHT_UBO));
        //cameraUbo
        setLinkage(cameraSource.addressOf(CameraSourcePass.CAMERA_UBO), deferredQueue.addressOf(SceneQueuePass.CAMERA_UBO));
        setLinkage(deferredQueue.addressOf(SceneQueuePass.CAMERA_UBO), lightPass.addressOf(LightPass.CAMERA_UBO));
        setLinkage(lightPass.addressOf(LightPass.CAMERA_UBO), forwardQueue.addressOf(SceneQueuePass.CAMERA_UBO));
    }

    public Camera getCamera() {
        return camera;
    }

    public String getDeferredQueueName() {
        return DEFERRED_QUEUE;
    }

    public String getForwardQueueName() {
        return FORWARD_QUEUE;
    }
}
