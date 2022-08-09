package testing;

import marshal.*;

public class MarshalTesting {

    public static void main(String[] args) {

        MarshalEntity e1 = Marshal.createEntity();
        e1.add(new B());
        MarshalContext context = new MarshalContext();
        Marshal.addContext(context);


        MarshalComponentMapper<A> mp = MarshalComponentMapper.getFor(A.class);

        Marshal.start();

        context.add(e1);

        System.out.println("main mapper.get:" + mp.get(e1));

        System.out.println("main family:"+Marshal.getEntities(MarshalFamily.instanceOf(A.class)));

    }

    private static class A extends MarshalComponent {

    }

    private static class B extends A {

    }
}
