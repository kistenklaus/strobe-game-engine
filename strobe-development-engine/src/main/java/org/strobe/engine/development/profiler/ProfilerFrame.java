package org.strobe.engine.development.profiler;

import java.util.ArrayList;

public final class ProfilerFrame {

    protected final String name;
    protected long start;
    protected long end = -1;

    private final ArrayList<ProfilerFrame> children = new ArrayList<>();

    public ProfilerFrame(String name) {
        this.start = System.nanoTime();
        this.name = name;
    }

    public void close() {
        end = System.nanoTime();
    }

    public void addChild(ProfilerFrame frame) {
        children.add(frame);
    }

    public long getStart() {
        return start;
    }

    public long getEnd() {
        return end;
    }

    public long getDuration() {
        return end - start;
    }

    public String getName() {
        return name;
    }

    public Iterable<ProfilerFrame> children() {
        return children;
    }
}
