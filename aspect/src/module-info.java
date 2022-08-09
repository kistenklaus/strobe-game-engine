module aspect {
    exports org.aspect;
    exports org.aspect.graphics.bindable;
    exports org.aspect.graphics.renderer;
    exports org.aspect.graphics.renderer.graph;
    exports org.aspect.graphics.passes;
    exports org.aspect.graphics.passes.queues;
    exports org.aspect.graphics.drawable;
    exports org.aspect.graphics.drawable.meshes;
    exports org.aspect.graphics.drawable.attachments;
    exports org.aspect.graphics.drawable.techniques;
    exports org.aspect.graphics.drawable.steps;
    exports org.aspect.window;
    exports org.aspect.window.event;
    exports org.aspect.window.listener;

    exports org.aspect.graphics.data;
    exports org.aspect.graphics.primitives;

    requires org.lwjgl;
    requires org.lwjgl.natives;
    requires org.lwjgl.glfw;
    requires org.lwjgl.glfw.natives;
    requires org.lwjgl.opengl;
    requires org.lwjgl.opengl.natives;
    requires org.lwjgl.opengles;
    requires org.lwjgl.opengles.natives;

    requires transitive org.joml;

    requires org.lwjgl.stb;
    requires org.lwjgl.stb.natives;

}