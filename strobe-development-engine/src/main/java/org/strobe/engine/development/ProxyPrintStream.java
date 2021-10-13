package org.strobe.engine.development;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.util.function.Consumer;

public final class ProxyPrintStream extends PrintStream {

    private final PrintStream prev;
    private Consumer<String> out = null;

    public ProxyPrintStream() {
        super(new ByteArrayOutputStream());
        prev = System.out;
        System.setOut(this);
    }

    public void setOut(Consumer<String> out){
        this.out = out;
    }

    @Override
    public void print(String s) {
        prev.print(s);
        if(out!=null)out.accept(s);
    }

    @Override
    public void println(String x) {
        prev.println(x);
        if(out!=null)out.accept(x);
    }

    public void close() {
        System.setOut(prev);
        close();
    }


    @Override
    public void print(boolean b) {
        print(Boolean.toString(b));
    }

    @Override
    public void print(char c) {
        print(Character.toString(c));
    }

    @Override
    public void print(int i) {
        print(Integer.toString(i));
    }

    @Override
    public void print(long l) {
        print(Long.toString(l));
    }

    @Override
    public void print(float f) {
        print(Float.toString(f));
    }

    @Override
    public void print(double d) {
        print(Double.toString(d));
    }

    @Override
    public void print(char[] s) {
        print(new String(s));
    }


    @Override
    public void print(Object obj) {
        print(obj.toString());
    }

    @Override
    public void println(boolean x) {
        println(Boolean.toString(x));
    }

    @Override
    public void println(char x) {
        println(Character.toString(x));
    }

    @Override
    public void println(int x) {
        println(Integer.toString(x));
    }

    @Override
    public void println(long x) {
        println(Long.toString(x));
    }

    @Override
    public void println(float x) {
        println(Float.toString(x));
    }

    @Override
    public void println(double x) {
        println(Double.toString(x));
    }

    @Override
    public void println(char[] x) {
        println(new String(x));
    }

    @Override
    public void println(Object x) {
        println(x.toString());
    }
}
