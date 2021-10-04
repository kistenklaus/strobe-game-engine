package org.strobe.game;

import org.joml.Quaternionf;
import org.joml.Vector3f;
import org.strobe.core.Strobe;
import org.strobe.debug.imgui.ImGuiDebugger;
import org.strobe.ecs.Entity;
import org.strobe.ecs.context.*;
import org.strobe.ecs.context.renderer.camera.CameraRenderer;
import org.strobe.ecs.context.renderer.camera.FocusCamera;
import org.strobe.ecs.context.renderer.camera.PerspectiveLens;
import org.strobe.ecs.context.renderer.light.DirectionalLight;
import org.strobe.ecs.context.renderer.materials.LambertianMaterial;
import org.strobe.ecs.context.renderer.mesh.Mesh;
import org.strobe.ecs.context.renderer.mesh.MeshRenderer;
import org.strobe.ecs.context.renderer.mesh.Primitive;
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
        Mesh mesh = new Mesh(Primitive.CUBE,Mesh.ALLOCATE_POSITIONS|Mesh.ALLOCATE_NORMALS|Mesh.ALLOCATE_TEXTURE_COORDS);

        renderable.addComponent(mesh);
        LambertianMaterial mat = new LambertianMaterial(1,0,1);
        mat.setDiffuseColor(new Vector3f(1,1,0));
        renderable.addComponent(mat);
        renderable.addComponent(new MeshRenderer());


        Entity model = ecs.createEntity();
        //model.addComponent(new Transform(new Vector3f(0,5,0)));
        //model.addComponent(new ModelLoaderScript("assets/backpack/backpack.obj"));

        Entity ground = ecs.createEntity();
        //ground.addComponent(new GroundGenScript());


        Entity camera = ecs.createEntity();
        camera.addComponent(new PerspectiveLens(60, 960f/640, 0.01f, 100));
        int resRed = 1;
        CameraRenderer cr;
        camera.addComponent(cr=new CameraRenderer(960/resRed, 640/resRed));
        cr.disableShadows();
        cr.enableFXAA();
        camera.addComponent(new CameraIndex(1));
        camera.addComponent(new Transform(new Vector3f(0,4f,5f), new Vector3f(1), new Quaternionf().identity()));
        camera.addComponent(new CameraController());
        camera.addComponent(new FocusCamera());


        Entity light = ecs.createEntity();
        light.addComponent(new Transform(new Vector3f(1,2,0)));
        DirectionalLight dirLightComp = new DirectionalLight(new Vector3f(0.1f), new Vector3f(1), new Vector3f(1));
        dirLightComp.setShadowFrustumOffset(10);
        light.addComponent(dirLightComp);
        light.addComponent(new DaylightCycle());

        Entity camera2 = ecs.createEntity();
        camera2.addComponent(new Transform(new Vector3f(0,0,5)));
        camera2.addComponent(new CameraIndex(2));
        camera2.addComponent(new CameraController());
        camera2.addComponent(new PerspectiveLens(60, 960f/640f, 0.01f, 10));
        camera2.addComponent(new CameraRenderer(960, 640));
        camera2.addComponent(new LookAtCenter());

        ecs.addEntitySystem(new CameraControllerSystem(ecs, getMouse(), getKeyboard()));
        ecs.addEntitySystem(new CameraFocusChanger(ecs, getKeyboard()));
        ecs.addEntitySystem(new DaylightSystem(ecs, getKeyboard()));
    }
}