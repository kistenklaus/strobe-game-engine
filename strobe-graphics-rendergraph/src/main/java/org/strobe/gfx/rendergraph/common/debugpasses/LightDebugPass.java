package org.strobe.gfx.rendergraph.common.debugpasses;

import org.joml.Matrix4f;
import org.joml.Quaternionf;
import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.camera.FrustumBox;
import org.strobe.gfx.lights.DirectionalLight;
import org.strobe.gfx.lights.ShadowUbo;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.shader.Shader;
import org.strobe.gfx.opengl.bindables.shader.io.ShaderLoader;
import org.strobe.gfx.opengl.bindables.vao.Ibo;
import org.strobe.gfx.opengl.bindables.vao.Vao;
import org.strobe.gfx.renderables.opengl.IndexedVao;
import org.strobe.gfx.rendergraph.common.manager.CameraManager;
import org.strobe.gfx.rendergraph.common.manager.LightManager;
import org.strobe.gfx.rendergraph.core.RenderPass;
import org.strobe.gfx.rendergraph.core.Resource;

import java.util.Arrays;

import static org.lwjgl.opengl.GL11.*;

public final class LightDebugPass extends RenderPass {

    private static final float DIR_LIGHT_SIZE = 0.1f;
    private static final float RAY_LENGTH = 0.5f;

    private final Resource<LightManager> lights = registerResource(LightManager.class, "lights");
    private final Resource<CameraManager> cameras = registerResource(CameraManager.class, "cameras");
    private final Resource<Framebuffer> target = registerResource(Framebuffer.class, "target");

    private final Shader debugShader;
    private final Uniform<Matrix4f> proj;
    private final Uniform<Matrix4f> view;
    private final Uniform<Matrix4f> model;

    private final IndexedVao dirLightVao;


    private final Vao shadowFrustumVao;
    private final IndexedVao shadowFrustum;

    public LightDebugPass(Graphics gfx) {
        debugShader = ShaderLoader.loadShader(gfx, "shaders/debugLine/");
        view = debugShader.getUniform(Matrix4f.class, "view");
        model = debugShader.getUniform(Matrix4f.class, "model");
        proj = debugShader.getUniform(Matrix4f.class, "proj");


        final int precision = 15;

        int[] indices = new int[precision*2 *2];
        for(int i=0;i<precision-1;i++){
            indices[i*2] = i;
            indices[i*2+1] = i+1;
        }
        indices[precision*2-2] = precision-1;
        indices[precision*2-1] = 0;
        for(int i=0;i<precision;i++){
            indices[i*2 + precision*2] = indices[i*2];
            indices[i*2+1 + precision*2] = indices[i*2] + precision;
        }


        Vao vao = new Vao(gfx, precision*3*2, new Ibo(gfx, indices, true),
                "layout(location=0,usage=static) in vec3");
        float[] circlePos = new float[precision*3*2];
        float delta = (float) (Math.PI*2 / precision);
        float theta = 0;
        for(int i=0;i<precision;i++){
            circlePos[i*3] = (float) Math.cos(theta);
            circlePos[i*3+1] = (float) Math.sin(theta);
            circlePos[i*3+2] = 0f;
            circlePos[i*3+3*precision] = circlePos[i*3];
            circlePos[i*3+1+3*precision] = circlePos[i*3+1];
            circlePos[i*3+2+3*precision] = -RAY_LENGTH;
            theta+=delta;
        }
        vao.bufferLocation(gfx, 0, circlePos);
        dirLightVao = new IndexedVao(vao, precision*2*2, GL_LINES);




        shadowFrustumVao = new Vao(gfx, 8, new Ibo(gfx, new int[]{
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
        }, true),"layout(location=0,usage=static) in vec3");
        shadowFrustum = new IndexedVao(shadowFrustumVao, 24, GL_LINES);
    }

    @Override
    protected void complete(Graphics gfx) {
        if (lights.get() == null) throw new IllegalStateException();
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
        gfx.bind(target.get());
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glLineWidth(2f);
        gfx.bind(debugShader);
        proj.set(gfx, mainCam.getProjMatrix());
        view.set(gfx, mainCam.getViewMatrix());
        Matrix4f model = new Matrix4f().identity();
        for (DirectionalLight dirLight : lights.get().directionalLights()) {
            model.identity();
            Vector3f pos = dirLight.getPosition();
            //fixme weird roll (learn Quaternions bitch)
            Quaternionf orientation = new Quaternionf().rotateTo(
                    0, 0, 1, pos.x, pos.y, pos.z);
            model.translate(pos);
            model.rotate(orientation);
            model.scale(DIR_LIGHT_SIZE, DIR_LIGHT_SIZE, 1);
            this.model.set(gfx, model);
            dirLightVao.render(gfx);
        }

        model.identity();
        this.model.set(gfx, model);
        for(int i=0;i<lights.get().shadowCameraCount();i++){
            ShadowUbo shadowUbo = lights.get().shadowUbos()[i];
            Matrix4f[] dirLightSpaces = shadowUbo.getDirLightSpaces();

            for(int j=0;j<shadowUbo.getDirLightCastingCount();j++){
                Matrix4f lightSpace = dirLightSpaces[j];
                FrustumBox frustum = FrustumBox.getFromProjView(lightSpace);
                shadowFrustumVao.bufferLocation(gfx, 0, frustum.toFloatArray_vec3aligned());
                shadowFrustum.render(gfx);
            }

        }

        gfx.unbind(debugShader);
        gfx.unbind(target.get());
    }

    @Override
    protected void dispose(Graphics gfx) {

    }

    public Resource<LightManager> getLightResource() {
        return lights;
    }

    public Resource<CameraManager> getCameraResource() {
        return cameras;
    }

    public Resource<Framebuffer> getTargetResource() {
        return target;
    }
}
