module aspect {
    exports org.aspect;
    exports org.aspect.graphics.bindable;
    exports org.aspect.graphics.bindable.shader;
    exports org.aspect.graphics.renderer;
    exports org.aspect.graphics.primitive;
    exports org.aspect.graphics.renderer.drawable;
    exports org.aspect.graphics.renderer.graph;
    exports org.aspect.graphics.renderer.graph.passes;
    exports org.aspect.graphics.window;
    exports org.aspect.graphics.window.event;
    exports org.aspect.graphics.window.listener;



    requires org.lwjgl;
    requires org.lwjgl.natives;
    requires org.lwjgl.glfw;
    requires org.lwjgl.glfw.natives;
    requires org.lwjgl.opengl;
    requires org.lwjgl.opengl.natives;
    requires org.lwjgl.opengles;
    requires org.lwjgl.opengles.natives;
    //requires org.lwjgl.assimp;

    requires transitive org.joml;

    requires org.lwjgl.stb;
    requires org.lwjgl.stb.natives;

}