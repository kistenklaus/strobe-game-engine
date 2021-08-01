package org.strobe.game;

import org.strobe.core.Strobe;
import org.strobe.ecs.Entity;
import org.strobe.ecs.context.*;
import org.strobe.ecs.context.renderer.Mesh;
import org.strobe.ecs.context.renderer.MeshRenderer;
import org.strobe.ecs.context.renderer.Transform;
import org.strobe.ecs.context.renderer.materials.TestMaterial;
import org.strobe.gfx.Graphics;
import org.strobe.window.glfw.GlfwWindow;

public class Game extends EntityContext {

    public static void main(String[] args) {
        Strobe.start(new Game(), new GlfwWindow("game-window", 960, 640));
    }

    @Override
    public void setup(Graphics gfx) {
        //addDebugger(new ImGuiDebugger(gfx))


        Entity renderable = ecs.createEntity();
        renderable.addComponent(new Transform());
        Mesh mesh = new Mesh(4, 6);
        mesh.setPositions(0, new float[]{-0.5f, -0.5f, 0f, -0.5f, 0.5f, 0f, 0.5f, 0.5f, 0f, 0.5f, -0.5f, 0f});
        mesh.setIndices(0, new int[]{2, 1, 0, 0, 3, 2});
        renderable.addComponent(mesh);
        TestMaterial mat;
        renderable.addComponent(mat = new TestMaterial());
        mat.switchShader();
        renderable.addComponent(new MeshRenderer());


        ecs.addEntitySystem(new TestMeshSystem(ecs,getKeyboard()));
    }
}
