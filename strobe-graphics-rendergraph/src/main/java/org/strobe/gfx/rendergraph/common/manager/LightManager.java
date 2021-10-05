package org.strobe.gfx.rendergraph.common.manager;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.AbstractCamera;
import org.strobe.gfx.camera.FrustumBox;
import org.strobe.gfx.lights.*;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;

import java.util.ArrayList;
import java.util.HashMap;

public final class LightManager {

    private final LightUbo ubo;
    private ArrayList<DirectionalLight> directionalLights = new ArrayList<>();

    private ArrayList<AbstractCamera> shadowCameras = new ArrayList<>();
    private final HashMap<AbstractCamera, Integer> cameraIndices = new HashMap<>();
    private final ShadowUbo[] shadowUbos = new ShadowUbo[LightConstants.MAX_SHADOW_RENDERER];
    private final Framebuffer[] shadowMaps = new Framebuffer[LightConstants.MAX_SHADOW_RENDERER];
    private int dirCasterCount;


    public LightManager(Graphics gfx) {
        ubo = new LightUbo(gfx);
        for (int i = 0; i < shadowUbos.length; i++) {
            shadowUbos[i] = new ShadowUbo(gfx);
            shadowMaps[i] = new Framebuffer(gfx, LightConstants.SHADOW_MAP_RESOLUTION,
                    LightConstants.SHADOW_MAP_RESOLUTION, Framebuffer.Attachment.SHADOW_DEPTH_ATTACHMENT);
        }
    }

    public void submitLight(AbstractLight light) {
        if (light.getClass() == DirectionalLight.class) {
            if (directionalLights.size() >= LightConstants.DIRECTIONAL_LIGHT_COUNT) return;
            directionalLights.add((DirectionalLight) light);
            if (light.isShadowCasting()) dirCasterCount++;
            return;
        }
        throw new UnsupportedOperationException();
    }

    public void submitShadowCamera(AbstractCamera camera) {
        if (shadowCameras.size() >= LightConstants.MAX_SHADOW_RENDERER) return;
        cameraIndices.put(camera, shadowCameras.size());
        this.shadowCameras.add(camera);
    }

    public void updateUbos(Graphics gfx) {
        ubo.uniformDirectionalLight(gfx, directionalLights);

        for (int i = 0; i < shadowCameras.size(); i++) {
            AbstractCamera shadowCamera = shadowCameras.get(i);
            FrustumBox frustum = shadowCamera.getFrustumBox();
            ShadowUbo shadowUbo = shadowUbos[i];
            Matrix4f[] lightSpaces = new Matrix4f[dirCasterCount];
            Vector4f[] shadowDims = new Vector4f[dirCasterCount];
            int[] indices = new int[LightConstants.DIRECTIONAL_LIGHT_COUNT];
            int k = 0, j;
            for (j = 0; j < directionalLights.size() && k < dirCasterCount; j++) {
                DirectionalLight dirLight = directionalLights.get(j);
                if (dirLight.isShadowCasting()) {
                    indices[j] = k;
                    //defines the region of the shadow map where the data for this light is stored
                    shadowDims[k] = new Vector4f(0, 0, 1, 1);
                    //TODO implement proper light space adjustment based on camera frustum box.

                    //creates a view matrix that sits at the center of the camera view frustum(cuboid)
                    //and looks in the direction of the directional light.
                    Matrix4f lightView = new Matrix4f().lookAt(dirLight.getPosition(),
                            //dir (0,1,0) or any other also works for the up vector
                            new Vector3f(0), shadowCamera.getViewMatrix().transformPosition(new Vector3f(0, 1, 0)));
                    lightView.translate(dirLight.getPosition());
                    Vector3f viewFrustumCenter = frustum.calculateCenter();
                    lightView.translate(-viewFrustumCenter.x, -viewFrustumCenter.y, -viewFrustumCenter.z);

                    FrustumBox lightSpaceFrustum = frustum.transform(lightView);
                    float[] lsFrustumArray = lightSpaceFrustum.toFloatArray_vec3aligned();
                    float minX = Float.POSITIVE_INFINITY;
                    float maxX = Float.NEGATIVE_INFINITY;
                    float minY = Float.POSITIVE_INFINITY;
                    float maxY = Float.NEGATIVE_INFINITY;
                    float minZ = Float.POSITIVE_INFINITY;
                    float maxZ = Float.NEGATIVE_INFINITY;

                    for (int l = 0; l < lsFrustumArray.length; l += 3) {
                        minX = Math.min(lsFrustumArray[l], minX);
                        maxX = Math.max(lsFrustumArray[l], maxX);

                        minY = Math.min(lsFrustumArray[l + 1], minY);
                        maxY = Math.max(lsFrustumArray[l + 1], maxY);

                        minZ = Math.min(lsFrustumArray[l + 2], minZ);
                        maxZ = Math.max(lsFrustumArray[l + 2], maxZ);
                    }


                    /*
                    float texelSize = 1.0f/LightConstants.SHADOW_MAP_RESOLUTION;
                    minX = (float)Math.ceil(minX / texelSize) * texelSize;
                    maxX = (float)Math.ceil(maxX / texelSize) * texelSize;
                    minY = (float)Math.ceil(minY / texelSize) * texelSize;
                    maxY = (float)Math.ceil(maxY / texelSize) * texelSize;
                     */


                    //TODO add shadow near plane offset to account for objects that are behind the near plane to cast shadows
                    Matrix4f lightProj = new Matrix4f().ortho(minX, maxX, minY, maxY, -maxZ - dirLight.getShadowFrustumOffset(), -minZ);
                    lightSpaces[k] = lightProj.mul(lightView);
                    k++;
                } else {
                    indices[j] = -1;
                }
            }
            for (int J = j; J < LightConstants.DIRECTIONAL_LIGHT_COUNT; J++) {
                indices[J] = -1;
            }
            shadowUbo.uniformDirLightLightSpaces(gfx, lightSpaces);

            shadowUbo.uniformDirLightShadowDims(gfx, shadowDims);
            shadowUbo.uniformDirLightIndices(gfx, indices);
            shadowUbo.uniformDirLightCastingCount(gfx, dirCasterCount);


        }
    }

    public ShadowUbo getCameraShadowUbo(AbstractCamera shadowCamera) {
        Integer index = cameraIndices.get(shadowCamera);
        if (index == null) return null;
        return shadowUbos[index];
    }

    public Framebuffer getCameraShadowMap(AbstractCamera shadowCamera) {
        Integer index = cameraIndices.get(shadowCamera);
        if (index == null) return null;
        return shadowMaps[index];
    }

    public int getDirCasterCount() {
        return dirCasterCount;
    }

    public int getDirLightCount() {
        return directionalLights.size();
    }

    public int getShadowCameraCount() {
        return shadowCameras.size();
    }

    /**
     * @param index
     * @return
     * @deprecated use getCameraShadowUbo instead
     */
    public Framebuffer getShadowMapByIndex(int index) {
        return shadowMaps[index];
    }

    /**
     * @param index
     * @return
     * @deprecated use getCameraShadowMap instead
     */
    public ShadowUbo getShadowUboByIndex(int index) {
        return shadowUbos[index];
    }

    public Iterable<DirectionalLight> directionalLights() {
        return directionalLights;
    }

    public void clearFrame() {
        directionalLights.clear();
        shadowCameras.clear();
        cameraIndices.clear();
        dirCasterCount = 0;
    }

    public LightUbo ubo() {
        return ubo;
    }

    public ShadowUbo[] shadowUbos() {
        return shadowUbos;
    }

    public int shadowCameraCount() {
        return shadowCameras.size();
    }
}
