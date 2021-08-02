package org.strobe.gfx.rendergraph.common;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
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

    private final Shader debugShader;

    private final Uniform<Matrix4f> proj;
    private final Uniform<Matrix4f> view;
    private final Uniform<Matrix4f> model;

    private final IndexedVao cameraVao;

    public CameraDebugPass(Graphics gfx) {
        debugShader = ShaderLoader.loadShader(gfx, "shaders/cameraDebug/");
        view = debugShader.getUniform(Matrix4f.class, "view");
        model = debugShader.getUniform(Matrix4f.class, "model");
        proj = debugShader.getUniform(Matrix4f.class, "proj");

        Vao vao = new Vao(gfx, 8, new Ibo(gfx, new int[]{
                0,1,
                1,2,
                2,3,
                3,0,

                4,5,
                5,6,
                6,4,

                0,7,
                1,7,
                2,7,
                3,7
        }, true), "layout(location=0,usage=static) in vec3");
        vao.bufferLocation(gfx, 0, new float[]{
                -1f, -1f, 0f,
                -1f, 1f, 0f,
                1f, 1f, 0f,
                1f, -1f, 0f,

                -0.5f, 1.1f, 0f,
                0.5f, 1.1f, 0f,
                0f, 1.5f, 0f,

                0,0, 1f,
        });
        cameraVao = new IndexedVao(vao, 22, GL_LINES);

    }

    @Override
    protected void complete(Graphics gfx) {
        if (cameras.get() == null) throw new IllegalStateException();
    }

    @Override
    protected void reset(Graphics gfx) {

    }

    @Override
    protected void render(Graphics gfx) {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        gfx.bind(debugShader);
        for (AbstractCamera camera : cameras.get().cameras()) {
            proj.set(gfx, camera.getProjMatrix());
            view.set(gfx, camera.getViewMatrix());
            gfx.bind(camera.getTarget());
            System.out.println("view" + camera.getViewMatrix());
            for (AbstractCamera toDebug : cameras.get().cameras()) {
                if (camera == toDebug) continue;
                Matrix4f model = new Matrix4f(toDebug.getViewMatrix());
                model.invertAffine();
                model.scale(new Vector3f(CAMERA_SIZE * camera.getAspect(), CAMERA_SIZE,
                        (float)Math.log1p(Math.log1p(camera.getFar()-camera.getNear()))));
                this.model.set(gfx, model);
                cameraVao.render(gfx);
            }
            gfx.unbind(camera.getTarget());
        }
        gfx.unbind(debugShader);
    }

    @Override
    protected void dispose(Graphics gfx) {

    }

    public final Resource<CameraManager> getCameraResource() {
        return cameras;
    }
}
