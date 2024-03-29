package org.strobe.game;

import org.joml.Quaternionf;
import org.joml.Vector3f;
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
import org.strobe.entry.Strobe;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.gfx.opengl.bindables.texture.TextureLoader;

public class Game extends EntityContext {

    public static void main(String[] args) {
        Strobe.start(Game.class, args);
    }

    public Game(){
        super("game-window", 1020, 720);
    }

    @Override
    public void setup(Graphics gfx) {


        Entity renderable = ecs.createEntity("renderable");
        renderable.addComponent(new Transform(new Vector3f(0,0.5f,0)));
        Mesh mesh = new Mesh(Primitive.CUBE,Mesh.ALLOCATE_POSITIONS|Mesh.ALLOCATE_NORMALS|Mesh.ALLOCATE_TEXTURE_COORDS);
        renderable.addComponent(mesh);

        LambertianMaterial mat = new LambertianMaterial(1,0,1);
        mat.setDiffuseColor(new Vector3f(1,1,0));
        Texture2D dtex = TextureLoader.loadTexture(gfx, "assets/BallsOfSteel.png");
        mat.setDiffuseTexture(dtex);


        renderable.addComponent(mat);
        renderable.addComponent(new MeshRenderer());

        Entity c1 = renderable.createChild("renderable-child");
        Entity c2 = c1.createChild("renderable-child-child");

        Entity ground = ecs.createEntity("ground");
        ground.addComponent(new GroundGenScript());


        Entity camera = ecs.createEntity("camera");
        camera.addComponent(new PerspectiveLens(60, 960f/640, 0.01f, 40));
        int resRed = 1;
        CameraRenderer cr;
        camera.addComponent(cr=new CameraRenderer(960/resRed, 640/resRed));
        cr.enableFXAA();
        camera.addComponent(new CameraIndex(1));
        camera.addComponent(new Transform(new Vector3f(0,4f,5f), new Vector3f(1), new Quaternionf().identity()));
        camera.addComponent(new CameraController());
        camera.addComponent(new FocusCamera());


        Entity light = ecs.createEntity("light");
        light.addComponent(new Transform(new Vector3f(1,2,3)));
        DirectionalLight dirLightComp = new DirectionalLight(new Vector3f(0.1f), new Vector3f(1), new Vector3f(1));
        dirLightComp.setShadowFrustumOffset(10);
        light.addComponent(dirLightComp);
        light.addComponent(new DaylightCycle());



        Entity light2 = ecs.createEntity("light2");
        light2.addComponent(new Transform(new Vector3f(1,2,-2)));
        dirLightComp = new DirectionalLight(new Vector3f(0.1f), new Vector3f(1), new Vector3f(1));
        dirLightComp.setShadowFrustumOffset(10);
        light2.addComponent(dirLightComp);

        Entity light3 = ecs.createEntity("light3");
        light3.addComponent(new Transform(new Vector3f(2,0.5f, -1)));
        dirLightComp = new DirectionalLight(new Vector3f(0.1f), new Vector3f(1), new Vector3f(1));
        dirLightComp.setShadowFrustumOffset(10);
        light3.addComponent(dirLightComp);

        Entity camera2 = ecs.createEntity("camera2");
        camera2.addComponent(new Transform(new Vector3f(0,0,5)));
        camera2.addComponent(new CameraIndex(2));
        camera2.addComponent(new CameraController());
        camera2.addComponent(new PerspectiveLens(60, 960f/640f, 0.01f, 5));
        camera2.addComponent(cr=new CameraRenderer(960, 640));
        cr.enableFXAA();
        camera2.addComponent(new LookAtCenter());

        ecs.addEntitySystem(new CameraControllerSystem(ecs, getMouse(), getKeyboard()));
        ecs.addEntitySystem(new CameraFocusChanger(ecs, getKeyboard()));
        ecs.addEntitySystem(new DaylightSystem(ecs, getKeyboard()));
    }
}