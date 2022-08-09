package aspect.testing;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.VertexArray;
import org.aspect.graphics.data.Material;
import org.aspect.graphics.drawable.Drawable;
import org.aspect.graphics.drawable.techniques.DeferredMaterialTechnique;
import org.aspect.graphics.drawable.techniques.LambertianMaterialTechnique;
import org.aspect.graphics.primitives.PrimitiveIbo;
import org.aspect.graphics.primitives.PrimitiveVbo;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.DeferredGraph;
import org.aspect.graphics.renderer.graph.GraphProfiler;
import org.aspect.graphics.renderer.DrawMethod;
import org.aspect.graphics.renderer.DrawMode;
import org.aspect.graphics.drawable.Transform;
import org.aspect.graphics.drawable.Mesh;
import org.aspect.window.Window;
import org.aspect.window.WindowAPI;
import org.aspect.window.listener.WindowKeyHandler;
import org.aspect.window.listener.WindowMouseHandler;
import org.joml.Vector3f;

public class AspectEntryTesting {

    public static void main(String[] args) {

        Window window = Aspect.createWindow("entry-testing",
                1280, 720, false, WindowAPI.GLFW);

        WindowKeyHandler keyHandler =
                window.setWindowKeyListener(new WindowKeyHandler());

        WindowMouseHandler mouseHandler =
                window.setWindowMouseListener(new WindowMouseHandler());


        Renderer renderer = window.getRenderer();

        DeferredGraph graph = new DeferredGraph(window.getRenderer(), window.getWidth(), window.getHeight());
        graph.setProfiler(new GraphProfiler());

        CameraController controller = new CameraController(graph.getCamera(),mouseHandler,keyHandler);
        renderer.addGraph(graph);

        renderer.link();
        renderer.complete();


        Material material = new Material(
                Aspect.loadTexture("assets/resources/diamond_ore.png", false),
                Aspect.loadTexture("assets/resources/diamond_ore_specular.png", false));


        VertexArray vao = Aspect.createVertexArray(true, new PrimitiveIbo.FillCube(),
                true, new PrimitiveVbo.CubePositions(5, 5, 5),
                new PrimitiveVbo.CubeUVs(), new PrimitiveVbo.CubeNormals());
        Mesh mesh = new Mesh(vao, DrawMethod.ELEMENTS, DrawMode.TRIANGLES);

        Transform transform1 = new Transform().setScale(new Vector3f(2)).setPosition(new Vector3f(5,0,0));
        Drawable defDrawable = new Drawable(mesh, transform1);
        //creating a deferred drawable
        defDrawable.addTechnique(new DeferredMaterialTechnique(graph.getDeferredQueueName(), material));


        Transform transform2 = new Transform().setScale(new Vector3f(2)).setPosition(new Vector3f(-10,0,0));
        Drawable forDrawable = new Drawable(mesh,transform2);
        //creating forward drawable
        forDrawable.addTechnique(new LambertianMaterialTechnique(graph.getForwardQueueName(),material));

        long last = System.nanoTime();

        int interval = 2500;
        int counter = interval;
        long timer_start = 0;
        long timer_end;

        while (!window.shouldClose()) {
            window.pollEvents();

            renderer.draw(defDrawable, graph);
            renderer.draw(forDrawable,graph);

            window.update();

            long curr = System.nanoTime();
            double dt = (curr - last) * 1e-9d;
            last = curr;

            counter++;
            if (counter > interval) {
                timer_end = System.nanoTime();
                long delta = timer_end - timer_start;
                System.out.println("FPS:" + interval / (delta * 1e-9f));
                graph.getProfiler().logProfile();
                graph.getProfiler().reset();
                counter = 0;
                timer_start = System.nanoTime();
            }

            controller.update(dt);

        }
    }
}
