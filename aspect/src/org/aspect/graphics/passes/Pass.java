package org.aspect.graphics.passes;

import org.aspect.graphics.renderer.graph.Sink;
import org.aspect.graphics.renderer.graph.Source;

import java.util.ArrayList;
import java.util.List;

public abstract class Pass {

    private final String name;

    private List<Sink<?>> sinks = new ArrayList<>();
    private List<Source<?>> sources = new ArrayList<>();

    public Pass(String name) {
        this.name = name;
    }

    public abstract void setupSinksAndSources();

    //getting around module visibility
    public abstract void complete();


    public abstract void execute();

    public void reset() {
    }

    private <T> Sink<T> registerSink(Sink<T> sink) {
        sink.registerTo(getName());
        sinks.add(sink);
        return sink;
    }

    private <T> Source<T> registerSource(Source<T> source) {
        source.registerTo(getName());
        sources.add(source);
        return source;
    }

    protected <T> Source<T> registerSource(Class<T> valueClass, String name, T value) {
        return registerSource(new Source<>(valueClass, name, value));
    }

    protected <T> Source<T> registerSinkSource(Class<T> valueClass, String name) {
        registerSink(new Sink(valueClass, name, name));
        return registerSource(new Source(valueClass, name));
    }

    public List<Sink<?>> getSinks() {
        return sinks;
    }

    public List<Source<?>> getSources() {
        return sources;
    }

    public String addressOf(String sinkOrSourceName) {
        return name + "." + sinkOrSourceName;
    }

    public String getName() {
        return name;
    }
}
