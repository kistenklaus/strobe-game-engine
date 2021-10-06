package org.strobe.entry;

import org.strobe.ecs.context.EntityContext;
import org.strobe.engine.StrobeContext;
import org.strobe.engine.StrobeEngine;
import org.strobe.engine.development.DevelopmentEngine;
import org.strobe.engine.release.ReleaseEngine;
import org.strobe.gfx.opengl.OpenGlContext;

import java.lang.reflect.InvocationTargetException;
import java.util.function.Function;

public final class Strobe {

    public static <T extends EntityContext> void start(Class<T> contextClass, String[] args) {
        //parse args
        EngineMode mode = EngineMode.RELEASE;
        for (String arg : args) {
            if (arg.startsWith("-")) {
                arg = arg.substring(1);
                if (arg.equals("development")) {
                    mode = EngineMode.DEVELOPMENT;
                } else if (arg.equals("release")) {
                    mode = EngineMode.RELEASE;
                }
            }
        }

        try {
            T context = contextClass.getConstructor().newInstance();
            mode.createEngineInstance(context);
            context.start();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
    }

    private enum EngineMode {
        DEVELOPMENT(DevelopmentEngine::new),
        RELEASE(ReleaseEngine::new);

        private Function<EntityContext, StrobeEngine> engineConstructor;

        EngineMode(Function<EntityContext, StrobeEngine> engineConstructor) {
            this.engineConstructor = engineConstructor;
        }

        public StrobeEngine createEngineInstance(EntityContext context) {
            return engineConstructor.apply(context);
        }
    }

}
