package aspect.testing;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.*;
import org.aspect.graphics.bindable.shader.ShaderIndexBuffer;
import org.aspect.graphics.bindable.shader.ShaderTexture;
import org.aspect.graphics.primitive.Primitive;
import org.aspect.graphics.renderer.DrawMode;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.drawable.Drawable;
import org.aspect.graphics.renderer.drawable.Step;
import org.aspect.graphics.renderer.drawable.Technique;
import org.aspect.graphics.renderer.graph.passes.BufferClearPass;
import org.aspect.graphics.window.Window;
import org.aspect.graphics.window.WindowAPI;
import org.aspect.graphics.window.listener.WindowEventHandler;
import org.aspect.graphics.window.listener.WindowKeyHandler;
import org.aspect.graphics.window.listener.WindowMouseHandler;
import org.joml.Vector4f;

public class AspectTesting {

    public static void main(String[] args) {
        //render initialization:
        Aspect.setGraphicsAPI("opengl");

        Window window =
                Aspect.createWindow("title",
                        800, 600, false, WindowAPI.GLFW);

        WindowKeyHandler keyHandler =
                window.setWindowKeyListener(new WindowKeyHandler());

        WindowMouseHandler mouseHandler =
                window.setWindowMouseListener(new WindowMouseHandler());

        WindowEventHandler windowHandler =
                window.setWindowListener(new WindowEventHandler());





        Renderer renderer = window.getRenderer();

        renderer.registerGlobalResource("back_buffer",
                Framebuffer.class, renderer.getBackBuffer());

        RenderQueue renderQueue = new RenderQueue("render-queue");
        renderer.attachPass(renderQueue);

        BufferClearPass clearPass = new BufferClearPass("buffer-clear",
                new Vector4f(0, 0.2f,0.5f,1));
        renderer.attachPass(clearPass);


        renderer.setLinkage("$.back_buffer", "buffer-clear.buffer");
        renderer.setLinkage("buffer-clear.buffer", "render-queue.target");
        renderer.setLinkage("render-queue.target", "$.back_buffer");

        renderer.link();
        renderer.complete();


        //Drawable setup:
        VertexArray vao = Aspect.createVertexArray();

        VertexBuffer vbo = Aspect.createVertexBuffer(
                new Primitive.QuadPositions(1,1), true);
        VertexBuffer uvVbo = Aspect.createVertexBuffer(
                new Primitive.QuadUVs(), true);

        IndexBuffer outlineIbo = Aspect.createIndexBuffer(new int[]{
                0, 1,
                1, 2,
                2, 3,
                3, 0
        }, 8, true);

        IndexBuffer fillIbo = Aspect.createIndexBuffer(new int[]{
                0, 1, 2,
                2, 3, 0
        }, 6, true);

        vao.setAttribute(vbo, 0, 3,
                3, 1, 0);
        vao.setAttribute(uvVbo, 1, 2,
                2, 1, 0);

        ShaderIndexBuffer shaderOutlineIbo = Aspect.wrapIndexBuffer(outlineIbo);
        ShaderIndexBuffer shaderFillIbo = Aspect.wrapIndexBuffer(fillIbo);


        Drawable drawable = new Drawable(vao);

        Technique technique = new Technique();

        Shader shader = Aspect.loadShader("assets/shaders/default.vs", "assets/shaders/default.fs");

        ShaderTexture texture = Aspect.loadWrappedTexture("assets/resources/diamond_ore.png",
                false, shader, "texture1");

        ShaderTexture texture2 = Aspect.loadWrappedTexture("assets/resources/gold_ore.png",
                false, shader, "texture2");

        Step step2 = new Step("render-queue", shader);
        step2.addBindable(shaderFillIbo);
        step2.addBindable(texture);
        step2.addBindable(texture2);
        step2.addPreparer((r, s) -> {
            r.setDrawMode(DrawMode.TRIANGLE);
            s.uniformVec4("color", new Vector4f(0, 0, 0, 1));
        });
        technique.addStep(step2);

        Step step = new Step("render-queue", shader);
        step.addBindable(shaderOutlineIbo);
        step.addPreparer((r, s) -> {
            r.setDrawMode(DrawMode.LINES);
            s.uniformVec4("color", new Vector4f(1, 0, 0, 1));
        });
        technique.addStep(step);

        drawable.addTechnique(technique);


        long last = System.nanoTime();

        while (!window.shouldClose()) {
            window.pollEvents();


            renderer.draw(drawable);

            renderer.render();

            window.swapBuffers();

            long curr = System.nanoTime();
            double dt = (curr - last) * 1e-9d;
            last = curr;
            //System.out.println(1 / dt);
        }
    }
}
