package aspect.testing;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.VertexArray;
import org.aspect.graphics.drawable.techniques.DeferredMaterialTechnique;
import org.aspect.graphics.drawable.techniques.ForwardMaterialTechnique;
import org.aspect.graphics.material.LambertianTexMaterial;
import org.aspect.graphics.primitives.PrimitiveIboContent;
import org.aspect.graphics.primitives.PrimitiveVboContent;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.DeferredGraph;
import org.aspect.graphics.renderer.graph.GraphProfiler;
import org.aspect.graphics.renderer.DrawMethod;
import org.aspect.graphics.renderer.DrawMode;
import org.aspect.graphics.drawable.Drawable;
import org.aspect.graphics.drawable.Transform;
import org.aspect.graphics.drawable.Mesh;
import org.aspect.window.Window;
import org.aspect.window.WindowAPI;
import org.aspect.window.listener.WindowKeyHandler;
import org.aspect.window.listener.WindowMouseHandler;
import org.joml.Vector3f;

import java.util.ArrayList;

public class DeferredGraphBenchmark {

    public static void main(String[] args) {

        Window window = Aspect.createWindow("entry-testing",
                1280, 720, true, WindowAPI.GLFW);

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


        //creating a deferred drawable
        VertexArray vao = Aspect.createVertexArray(true, new PrimitiveIboContent.FillCube(),
                true, new PrimitiveVboContent.CubePositions(5, 5, 5),
                new PrimitiveVboContent.CubeUVs(), new PrimitiveVboContent.CubeNormals());
        Mesh mesh = new Mesh(vao, DrawMethod.ELEMENTS, DrawMode.TRIANGLES);

        ArrayList<Drawable> scene = new ArrayList<>();

        LambertianTexMaterial material = new LambertianTexMaterial("assets/resources/image0.jpg", "assets/resources/image0.jpg");


        boolean deferred = false;
        float radius = 500;
        for (int i = 0; i < 2500; i++) {
            float scale = (float) (Math.random() * 5);
            Vector3f rot = new Vector3f(
                    (float) (Math.random() * 2 * Math.PI),
                    (float) (Math.random() * 2 * Math.PI),
                    (float) (Math.random() * 2 * Math.PI)
            );
            Vector3f pos = new Vector3f(
                    (float) Math.random() * radius * 2 - radius,
                    (float) Math.random() * radius * 2 - radius,
                    (float) (Math.random() * radius * 2 - radius)
            );
            Drawable drawable = new Drawable(mesh,new Transform().setScale(scale).setRotation(rot).setPosition(pos));
            double ran = Math.random();
            if (true) {
                drawable.addTechnique(new DeferredMaterialTechnique(graph.getDeferredQueueName(),material));
            } else if(ran<0.6666){
                drawable.addTechnique(new ForwardMaterialTechnique(graph.getAlphaQueueName(),material));
            }else{
                drawable.addTechnique(new ForwardMaterialTechnique(graph.getForwardQueueName(),material));
            }
            scene.add(drawable);

        }


        long last = System.nanoTime();

        int interval = 180;
        int counter = interval;
        long timer_start = 0;
        long timer_end;

        while (!window.shouldClose()) {
            window.pollEvents();

            for (Drawable drawable : scene) {
                renderer.draw(drawable, graph);
            }

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
