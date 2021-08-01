package org.strobe.debug;

import java.lang.reflect.Field;

public class ProcessedDebuggableClass {
    private boolean[] display;
    private boolean[] readOnly;
    private boolean[] accessible;
    private Field[] fields;

    ProcessedDebuggableClass(boolean[] display, boolean[] readOnly, boolean[] accessible, Field[] fields) {
        this.display = display;
        this.readOnly = readOnly;
        this.accessible = accessible;
        this.fields = fields;
    }

    public boolean[] getDisplay() {
        return display;
    }

    public boolean[] getReadOnly() {
        return readOnly;
    }

    public boolean[] getAccessible() {
        return accessible;
    }

    public Field[] getFields() {
        return fields;
    }
}
