package org.strobe.gfx.rendergraph.common.manager;

import org.joml.Matrix4f;
import org.joml.Vector4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.camera.CameraFrustum;
import org.strobe.gfx.lights.*;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.ubo.Ubo;

import java.util.ArrayList;

public final class LightManager {

    private ArrayList<AbstractCamera> shadowCameras = new ArrayList<>();

    private ArrayList<DirectionalLight> directionalLights = new ArrayList<>();
    private final LightUbo ubo;
    private final ShadowUbo[] shadowUbos = new ShadowUbo[LightConstants.MAX_SHADOW_RENDERER];
    private final Framebuffer[] shadowMaps = new Framebuffer[LightConstants.MAX_SHADOW_RENDERER];

    private int dirCasterCount;

    public LightManager(Graphics gfx){
        ubo = new LightUbo(gfx);
        for(int i=0;i<shadowUbos.length;i++){
            shadowUbos[i] = new ShadowUbo(gfx);
            shadowMaps[i] = new Framebuffer(gfx, LightConstants.SHADOW_MAP_RESOLUTION,
                    LightConstants.SHADOW_MAP_RESOLUTION, Framebuffer.Attachment.DEPTH_ATTACHMENT,
                    Framebuffer.Attachment.COLOR_RGBA_ATTACHMENT_0);
        }
    }

    public void submitLight(AbstractLight light){
        if(light.getClass() == DirectionalLight.class){
            if(directionalLights.size() >= LightConstants.DIRECTIONAL_LIGHT_COUNT)return;
            directionalLights.add((DirectionalLight) light);
            if(light.isShadowCasting()) dirCasterCount++;
            return;
        }
        throw new UnsupportedOperationException();
    }

    public void submitShadowCamera(AbstractCamera camera){
        if(shadowCameras.size()>= LightConstants.MAX_SHADOW_RENDERER)return;
        this.shadowCameras.add(camera);
    }

    public void updateUbos(Graphics gfx){
        ubo.uniformDirectionalLight(gfx, directionalLights);

        for(int i=0;i<shadowCameras.size();i++){
            AbstractCamera shadowCamera = shadowCameras.get(i);
            CameraFrustum frustum = shadowCamera.getFrustumBox();
            ShadowUbo ubo = shadowUbos[i];
            Matrix4f[] lightSpaces = new Matrix4f[dirCasterCount];
            Vector4f[] shadowDims = new Vector4f[dirCasterCount];
            int[] indices = new int[directionalLights.size()];
            int k =0;
            for(int j=0;j<directionalLights.size() && k<dirCasterCount;i++){
                DirectionalLight light = directionalLights.get(j);
                if(light.isShadowCasting()){
                    indices[j] = k;
                    //defines the region of the shadow map where the data for this light is stored
                    shadowDims[k] = new Vector4f(0,0,1,1);
                    //TODO implement proper light space adjustment based on camera frustum box.
                    Matrix4f lightView = new Matrix4f();

                    lightSpaces[k] = new Matrix4f().identity();
                    k++;
                }else{
                    indices[j] = -1;
                }
            }
            ubo.uniformDirLightLightSpaces(gfx, lightSpaces);
            ubo.uniformDirLightShadowDims(gfx, shadowDims);
            ubo.uniformDirLightIndices(gfx, indices);
            ubo.uniformDirLightCastingCount(gfx, dirCasterCount);

        }
    }

    public int getDirCasterCount(){
        return dirCasterCount;
    }

    public int getShadowCameraCount(){
        return shadowCameras.size();
    }

    public Framebuffer getShadowMap(int index){
        return shadowMaps[index];
    }

    public Ubo getShadowUbo(int index){
        return shadowUbos[index];
    }

    public Iterable<DirectionalLight> directionalLights(){
        return directionalLights;
    }

    public void clearFrame(){
        directionalLights.clear();
        shadowCameras.clear();
        dirCasterCount = 0;
    }

    public LightUbo ubo(){
        return ubo;
    }

}
