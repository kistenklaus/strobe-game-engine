package org.aspect.exceptions;

import org.aspect.graphics.window.Window;

public class AspectWindowException extends RuntimeException {
    public AspectWindowException(String errorMessage) {
        super(errorMessage);
    }

    public AspectWindowException(Window window, String errorMessage) {
        super((window != null ? "thrown from Window:[" + window.getTitle() + "]\n" : "") + errorMessage);
    }
}
