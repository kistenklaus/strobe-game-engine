package org.strobe.game;

import org.joml.Quaternionf;
import org.joml.Vector3f;
import org.strobe.core.Strobe;
import org.strobe.debug.imgui.ImGuiDebugger;
import org.strobe.ecs.Entity;
import org.strobe.ecs.context.*;
import org.strobe.ecs.context.renderer.camera.CameraRenderer;
import org.strobe.ecs.context.renderer.camera.FocusCamera;
import org.strobe.ecs.context.renderer.camera.PerspectiveLense;
import org.strobe.ecs.context.renderer.light.DirectionalLight;
import org.strobe.ecs.context.renderer.materials.LambertianMaterial;
import org.strobe.ecs.context.renderer.mesh.Mesh;
import org.strobe.ecs.context.renderer.mesh.MeshRenderer;
import org.strobe.ecs.context.renderer.transform.Transform;
import org.strobe.gfx.Graphics;
import org.strobe.window.glfw.GlfwWindow;

public class Game extends EntityContext {

    public static void main(String[] args) {
        Strobe.start(new Game(), new GlfwWindow("game-window", 960, 640));
    }

    @Override
    public void setup(Graphics gfx) {
        addDebugger(new ImGuiDebugger(gfx));


        Entity renderable = ecs.createEntity();
        renderable.addComponent(new Transform());
        Mesh mesh = new Mesh(4, 6);
        mesh.setPositions(0, new float[]{-0.5f, -0.5f, 0f, -0.5f, 0.5f, 0f, 0.5f, 0.5f, 0f, 0.5f, -0.5f, 0f});
        mesh.setNormals(0, new float[]{0,0,1,0,0,1,0,0,1,0,0,1});
        mesh.setIndices(0, new int[]{2, 1, 0, 0, 3, 2});
        renderable.addComponent(mesh);
        LambertianMaterial mat = new LambertianMaterial(1,0,1);
        mat.setDiffuseColor(new Vector3f(1,1,0));
        renderable.addComponent(mat);
        renderable.addComponent(new MeshRenderer());


        Entity camera = ecs.createEntity();
        camera.addComponent(new PerspectiveLense(60, 960f/640, 0.01f, 100));
        int resRed = 1;
        CameraRenderer cr;
        camera.addComponent(cr=new CameraRenderer(960/resRed, 640/resRed));
        //cr.enableFXAA();
        camera.addComponent(new Transform(new Vector3f(0,0f,5f), new Vector3f(1), new Quaternionf().identity()));
        camera.addComponent(new CameraController());
        camera.addComponent(new FocusCamera());


        Entity light = ecs.createEntity();
        light.addComponent(new Transform(new Vector3f(0,2,0)));
        light.addComponent(new DirectionalLight(new Vector3f(0.1f), new Vector3f(1), new Vector3f(1)));
        light.addComponent(new DaylightCycle());

        Entity c2 = light.createChild();
        c2.addComponent(new PerspectiveLense(60, 960/640f, 0.01f, 100f));
        c2.addComponent(new CameraRenderer(960, 640));
        c2.addComponent(new Transform(new Vector3f(0,0,1f), new Vector3f(1), new Quaternionf()));

        ecs.addEntitySystem(new TestMeshSystem(ecs,getKeyboard()));
        ecs.addEntitySystem(new CameraControllerSystem(ecs, getMouse(), getKeyboard()));
        ecs.addEntitySystem(new DaylightSystem(ecs));
    }
}
