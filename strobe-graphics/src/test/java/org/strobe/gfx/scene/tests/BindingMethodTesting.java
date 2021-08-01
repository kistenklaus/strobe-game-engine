package org.strobe.gfx.scene.tests;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.strobe.gfx.*;

import java.util.Collections;
import java.util.Iterator;

import static org.junit.jupiter.api.Assertions.assertEquals;

public final class BindingMethodTesting {

    private Graphics gfx;
    private StringBuilder str;

    @BeforeEach
    void beforeEach() {
        gfx = new Graphics(null);
        str = new StringBuilder();
    }

    @Test
    void testUnconditionalBinding() {
        Bindable a = new UnConBindable(gfx, "a", str);
        Bindable b = new UnConBindable(gfx, "b", str);
        Bindable c = new UnConBindable(gfx, "c", str);

        gfx.bind(a);
        gfx.bind(b);
        gfx.bind(c);
        gfx.unbind(a);
        gfx.bind(c);
        gfx.unbind(c);
        gfx.bind(b);
        gfx.bind(a);
        gfx.unbind(b);
        gfx.unbind(a);
        gfx.unbind(b);

        assertEquals(str.toString(), "bind[a]bind[b]bind[c]bind[c]bind[b]bind[a]");
    }

    @Test
    void testUnconditionalUnbinding() {
        Bindable a = new UnConUnbindable(gfx, "a", str);
        Bindable b = new UnConUnbindable(gfx, "b", str);
        Bindable c = new UnConUnbindable(gfx, "c", str);

        gfx.bind(a);
        gfx.bind(b);
        gfx.bind(c);
        gfx.unbind(a);
        gfx.bind(c);
        gfx.unbind(c);
        gfx.bind(b);
        gfx.bind(a);
        gfx.unbind(b);
        gfx.unbind(a);
        gfx.unbind(b);

        Assertions.assertEquals(str.toString(), "bind[a]bind[b]bind[c]unbind[a]bind[c]unbind[c]bind[b]bind[a]unbind[b]unbind[a]unbind[b]");
    }

    @Test
    void testConditionalBinding() {
        Bindable a = new ConBindable(gfx, "a", str);
        Bindable b = new ConBindable(gfx, "b", str);
        Bindable c = new ConBindable(gfx, "c", str);

        gfx.bind(a);        //bind a
        gfx.bind(b);        //bind b
        gfx.bind(c);        //bind c
        gfx.unbind(a);      //ignored
        gfx.bind(c);        //ignored
        gfx.unbind(c);      //ignored
        gfx.bind(b);        //bind b
        gfx.bind(a);        //bind a
        gfx.unbind(b);      //ignored
        gfx.unbind(a);      //ignored
        gfx.unbind(b);      //ignored

        Assertions.assertEquals(str.toString(), "bind[a]bind[b]bind[c]bind[b]bind[a]");
    }

    @Test
    void testConditionalUnbinding() {
        Bindable a = new ConUnbindable(gfx, "a", str);
        Bindable b = new ConUnbindable(gfx, "b", str);
        Bindable c = new ConUnbindable(gfx, "c", str);

        gfx.bind(a);        //bind a
        gfx.bind(b);        //unbind a -> bind b
        gfx.bind(c);        //unbind b -> bind c
        gfx.unbind(a);      //ignored
        gfx.bind(c);        //ignored
        gfx.unbind(c);      //unbind c      (prev = null)
        gfx.bind(b);        //bind b
        gfx.bind(a);        //unbind b -> bind a
        gfx.unbind(b);      //ignored
        gfx.unbind(a);      //unbind a      (prev = null)
        gfx.unbind(b);      //ignored

        System.out.println(str);

        Assertions.assertEquals(str.toString(), "bind[a]unbind[a]bind[b]unbind[b]bind[c]unbind[c]bind[b]unbind[b]bind[a]unbind[a]");
    }

    @Test
    void testStackBinding() {
        Bindable a = new StackBindable(gfx, "a", str);
        Bindable b = new StackBindable(gfx, "b", str);
        Bindable c = new StackBindable(gfx, "c", str);

        gfx.bind(a);    //bind a
        gfx.bind(b);    //bind b
        gfx.bind(c);    //bind c
        gfx.unbind(a);  //ignored
        gfx.bind(c);    //ignored
        gfx.unbind(c);  //(unbind c) -> bind b
        gfx.bind(b);    //ignored
        gfx.bind(a);    //bind a
        gfx.unbind(b);  //ignored
        gfx.unbind(a);  //(unbind a) -> bind b
        gfx.unbind(b);  //(unbind b) -> bind a
        gfx.unbind(a);  //(unbind a)

        Assertions.assertEquals(str.toString(), "bind[a]bind[b]bind[c]bind[b]bind[a]bind[b]bind[a]");
    }

    @Test
    void testStackUnbinding() {
        Bindable a = new StackUnbindable(gfx, "a", str);
        Bindable b = new StackUnbindable(gfx, "b", str);
        Bindable c = new StackUnbindable(gfx, "c", str);

        gfx.bind(a);    //bind a
        gfx.bind(b);    //unbind a -> bind b
        gfx.bind(c);    //unbind b -> bind c
        gfx.unbind(a);  //ignored
        gfx.bind(c);    //ignored
        gfx.unbind(c);  //unbind c -> bind b
        gfx.bind(b);    //ignored
        gfx.bind(a);    //unbind b -> bind a
        gfx.unbind(b);  //ignored
        gfx.unbind(a);  //unbind a -> bind b
        gfx.unbind(b);  //unbind b -> bind a
        gfx.unbind(a);  //unbind a

        Assertions.assertEquals(str.toString(), "bind[a]unbind[a]bind[b]unbind[b]bind[c]unbind[c]bind[b]unbind[b]" +
                "bind[a]unbind[a]bind[b]unbind[b]bind[a]unbind[a]");
    }


    //A FUCK TON OF TEST CLASSES.

    private static abstract class TBindable extends Bindable {
        StringBuilder str;
        private final String name;

        protected TBindable(Graphics gfx, String name, StringBuilder str) {
            super(gfx);
            this.name = name;
            this.str = str;
        }

        @Override
        protected void bind(Graphics gfx) {
            str.append("bind[" + name + "]");
        }

        @Override
        protected void unbind(Graphics gfx) {
            str.append("unbind[" + name + "]");
        }

        @Override
        protected void dispose(Graphics gfx) {
        }
    }

    @Pool(UnConBindingPool.class)
    private static class UnConBindable extends TBindable {
        protected UnConBindable(Graphics gfx, String name, StringBuilder str) {
            super(gfx, name, str);
        }
    }

    @Pool(UnConUnbindingPool.class)
    private static class UnConUnbindable extends TBindable {
        protected UnConUnbindable(Graphics gfx, String name, StringBuilder str) {
            super(gfx, name, str);
        }
    }

    @Pool(ConBindingPool.class)
    private static class ConBindable extends TBindable {
        protected ConBindable(Graphics gfx, String name, StringBuilder str) {
            super(gfx, name, str);
        }
    }

    @Pool(ConUnbindingPool.class)
    private static class ConUnbindable extends TBindable {
        protected ConUnbindable(Graphics gfx, String name, StringBuilder str) {
            super(gfx, name, str);
        }
    }

    @Pool(StackBindingPool.class)
    private static class StackBindable extends TBindable {
        protected StackBindable(Graphics gfx, String name, StringBuilder str) {
            super(gfx, name, str);
        }
    }

    @Pool(StackUnbindingPool.class)
    private static class StackUnbindable extends TBindable {
        protected StackUnbindable(Graphics gfx, String name, StringBuilder str) {
            super(gfx, name, str);
        }
    }

    public static class TBindingPool extends BindablePool<TBindable> {

        public TBindingPool(BindingMethod method) {
            super(method);
        }

        @Override
        public void dispose(Graphics gfx, TBindable bindable) {
        }

        @Override
        public Iterator<TBindable> iterator() {
            return Collections.emptyIterator();
        }
    }

    public static class UnConBindingPool extends TBindingPool {
        public UnConBindingPool() {
            super(new UnconditionalBinding());
        }
    }

    public static class UnConUnbindingPool extends TBindingPool {
        public UnConUnbindingPool() {
            super(new UnconditionalUnbinding());
        }
    }

    public static class ConBindingPool extends TBindingPool {
        public ConBindingPool() {
            super(new ConditionalBinding());
        }
    }

    public static class ConUnbindingPool extends TBindingPool {
        public ConUnbindingPool() {
            super(new ConditionalUnbinding());
        }
    }

    public static class StackBindingPool extends TBindingPool {
        public StackBindingPool() {
            super(new StackBinding());
        }
    }

    public static class StackUnbindingPool extends TBindingPool {
        public StackUnbindingPool() {
            super(new StackUnbinding());
        }
    }

}
