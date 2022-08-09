package org.aspect.exceptions;

public class AspectUnregisteredPassException extends RuntimeException{
    public AspectUnregisteredPassException(String errorMessage){
        super(errorMessage);
    }
}
