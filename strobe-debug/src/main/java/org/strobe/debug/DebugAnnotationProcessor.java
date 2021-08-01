package org.strobe.debug;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

public class DebugAnnotationProcessor {

    private static HashMap<Class<? extends Debuggable>, ProcessedDebuggableClass> parsedMap = new HashMap<>();

    public static ProcessedDebuggableClass process(Debuggable debuggable) {
        return process(debuggable.getClass());
    }

    public static ProcessedDebuggableClass process(Class<? extends Debuggable> debuggableClass) {
        ProcessedDebuggableClass mapValue = parsedMap.get(debuggableClass);
        if (mapValue != null) return mapValue;
        List<Field> fields = new ArrayList<>();
        List<Boolean> display = new ArrayList<>();
        List<Boolean> readOnly = new ArrayList<>();
        List<Boolean> accessible = new ArrayList<>();

        boolean direct = true;
        Class c = debuggableClass;
        while(c != Object.class){
            process(c, fields, display, readOnly, accessible, direct);
            direct = false;
            c = c.getSuperclass();
        }

        Field[] fieldArray = new Field[fields.size()];
        boolean[] displayArray = new boolean[display.size()];
        boolean[] readOnlyArray = new boolean[readOnly.size()];
        boolean[] accessibleArray = new boolean[accessible.size()];
        for (int i = 0; i < fields.size(); i++) {
            fieldArray[i] = fields.get(i);
            displayArray[i] = display.get(i);
            readOnlyArray[i] = readOnly.get(i);
            accessibleArray[i] = accessible.get(i);
        }

        ProcessedDebuggableClass parsedClass = new ProcessedDebuggableClass(displayArray, readOnlyArray,
                accessibleArray, fieldArray);
        parsedMap.put(debuggableClass, parsedClass);
        return parsedClass;
    }

    private static void process(Class<? extends Debuggable> debuggableClass, List<Field> fields,
                                List<Boolean> display, List<Boolean> readOnly, List<Boolean> accessible, boolean direct) {
        for (Field field : debuggableClass.getDeclaredFields()) {
            Debug fieldDebug = field.getAnnotation(Debug.class);
            if (fieldDebug != null) {
                fields.add(field);
                display.add(direct || fieldDebug.inherit());
                accessible.add(Modifier.isPublic(field.getModifiers()));
                readOnly.add(true); //TODO dependent on fieldDebug
            }
        }
    }

    private DebugAnnotationProcessor() {
        throw new UnsupportedOperationException();
    }

}
