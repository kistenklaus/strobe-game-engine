package org.strobe.gfx.rendergraph.common.debugpasses;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.camera.CameraFrustum;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.shader.Shader;
import org.strobe.gfx.opengl.bindables.shader.io.ShaderLoader;
import org.strobe.gfx.opengl.bindables.vao.Ibo;
import org.strobe.gfx.opengl.bindables.vao.Vao;
import org.strobe.gfx.renderables.opengl.IndexedVao;
import org.strobe.gfx.rendergraph.common.manager.CameraManager;
import org.strobe.gfx.rendergraph.core.RenderPass;
import org.strobe.gfx.rendergraph.core.Resource;

import static org.lwjgl.opengl.GL11.*;

public final class CameraDebugPass extends RenderPass {

    private static final float CAMERA_SIZE = 0.3f;

    private final Resource<CameraManager> cameras = registerResource(CameraManager.class, "cameras");
    private final Resource<Framebuffer> target = registerResource(Framebuffer.class, "target");

    private final Shader debugShader;

    private final Uniform<Matrix4f> proj;
    private final Uniform<Matrix4f> view;
    private final Uniform<Matrix4f> model;

    private final IndexedVao cameraVao;
    private final Vao frustumVao;

    public CameraDebugPass(Graphics gfx) {
        debugShader = ShaderLoader.loadShader(gfx, "shaders/debugLine/");
        view = debugShader.getUniform(Matrix4f.class, "view");
        model = debugShader.getUniform(Matrix4f.class, "model");
        proj = debugShader.getUniform(Matrix4f.class, "proj");

        Vao vao = new Vao(gfx, 8, new Ibo(gfx, new int[]{
                0, 1,
                1, 2,
                2, 3,
                3, 0,

                4, 5,
                5, 6,
                6, 4,

                0, 7,
                1, 7,
                2, 7,
                3, 7
        }, true), "layout(location=0,usage=static) in vec3");
        vao.bufferLocation(gfx, 0, new float[]{
                -1f, -1f, 0f,
                -1f, 1f, 0f,
                1f, 1f, 0f,
                1f, -1f, 0f,

                -0.5f, 1.1f, 0f,
                0.5f, 1.1f, 0f,
                0f, 1.5f, 0f,

                0, 0, 1f,
        });
        cameraVao = new IndexedVao(vao, 22, GL_LINES);

        frustumVao = new Vao(gfx, 8, new Ibo(gfx, new int[]{
                0, 1,
                1, 2,
                2, 3,
                3, 0,

                4, 5,
                5, 6,
                6, 7,
                7, 4,

                0, 4,
                1, 5,
                2, 6,
                3, 7
        }, true), "layout(location=0,usage=dynamic) in vec3");

    }

    @Override
    protected void complete(Graphics gfx) {
        if (cameras.get() == null) throw new IllegalStateException();
        if (target.get() == null) throw new IllegalStateException();
    }

    @Override
    protected void reset(Graphics gfx) {

    }

    @Override
    protected void render(Graphics gfx) {
        AbstractCamera mainCam = cameras.get().getSelectedCamera();
        if (mainCam == null) return;
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        gfx.bind(debugShader);
        proj.set(gfx, mainCam.getProjMatrix());
        view.set(gfx, mainCam.getViewMatrix());
        gfx.bind(target.get());
        for (AbstractCamera toDebug : cameras.get().cameras()) {
            if (mainCam == toDebug) continue;
            //update frustum Vao.
            CameraFrustum frustum = toDebug.getFrustumBox();

            frustumVao.bufferLocation(gfx, 0, new float[]{
                    frustum.nearBottomLeft().x, frustum.nearBottomLeft().y, frustum.nearBottomLeft().z,
                    frustum.nearTopLeft().x, frustum.nearTopLeft().y, frustum.nearTopLeft().z,
                    frustum.nearTopRight().x, frustum.nearTopRight().y, frustum.nearTopRight().z,
                    frustum.nearBottomRight().x, frustum.nearBottomRight().y, frustum.nearBottomRight().z,

                    frustum.farBottomLeft().x, frustum.farBottomLeft().y, frustum.farBottomLeft().z,
                    frustum.farTopLeft().x, frustum.farTopLeft().y, frustum.farTopLeft().z,
                    frustum.farTopRight().x, frustum.farTopRight().y, frustum.farTopRight().z,
                    frustum.farBottomRight().x, frustum.farBottomRight().y, frustum.farBottomRight().z
            });

            this.model.set(gfx, new Matrix4f().identity());
            gfx.bind(frustumVao);
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
            gfx.unbind(frustumVao);

            Matrix4f model = new Matrix4f(toDebug.getViewMatrix());
            model.invertAffine();
            model.scale(new Vector3f(CAMERA_SIZE * toDebug.getAspect(), CAMERA_SIZE,
                    (float) Math.log1p(Math.log1p(toDebug.getFar() - toDebug.getNear()))));
            this.model.set(gfx, model);
            cameraVao.render(gfx);
        }
        gfx.unbind(target.get());
        gfx.unbind(debugShader);
    }

    @Override
    protected void dispose(Graphics gfx) {

    }

    public final Resource<CameraManager> getCameraResource() {
        return cameras;
    }

    public final Resource<Framebuffer> getTargetResource() {
        return target;
    }
}
