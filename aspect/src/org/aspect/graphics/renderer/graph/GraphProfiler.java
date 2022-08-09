package org.aspect.graphics.renderer.graph;

import java.util.ArrayList;
import java.util.HashMap;

public class GraphProfiler {


    private long start;
    private long last;
    private int count = 0;
    private int totalEpi = 0;
    private HashMap<String, Long> epiMap = new HashMap<>();

    public void start() {
        start = System.nanoTime();
        last = start;
    }

    public void completed(String name) {
        long curr = System.nanoTime();
        long epi = curr - last;
        Long total = epiMap.get(name);
        if (total == null) total = 0l;
        epiMap.put(name, total + epi);
        last = System.nanoTime();
    }

    public void end() {
        totalEpi += System.nanoTime() - start;
        count++;
    }

    public void logProfile() {
        ArrayList<String> names = new ArrayList<>(epiMap.keySet());
        names.sort((a,b)->-Long.compare(epiMap.get(a),epiMap.get(b)));
        for (String passName : names) {
            Long epi = epiMap.get(passName);
            System.out.println(passName + " : " + epi / (float) count);
        }
        System.out.println("total : " + totalEpi / (float) count);
    }

    public void reset() {
        epiMap.clear();
        totalEpi = 0;
        count = 0;
    }
}
