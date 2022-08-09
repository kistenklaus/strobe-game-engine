package org.aspect.graphics.renderer.graph;

import org.aspect.datatype.Pair;
import org.aspect.exceptions.AspectGraphLinkingException;
import org.aspect.exceptions.AspectUnregisteredPassException;
import org.aspect.graphics.renderer.DrawingJob;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.passes.Pass;
import org.aspect.graphics.renderer.graph.passes.RenderQueuePass;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;

public class RenderGraph {

    private final HashMap<String, Pass> passes = new HashMap<>();
    private final ArrayList<Pass> passQueue = new ArrayList<>();

    private final HashMap<String, Sink<?>> globalSinks = new HashMap<>();
    private final HashMap<String, Source<?>> globalSources = new HashMap<>();

    //key : sources; value : sinks
    private final HashMap<String, String> namedLinks = new HashMap<>();

    private final Renderer renderer;

    public RenderGraph(Renderer renderer) {
        this.renderer = renderer;
    }

    public void addPass(Pass pass) {
        String passName = pass.getName();
        passes.put(passName, pass);
    }

    public <T> void registerGlobalResource(String name, Class<T> resourceClass, T resource) {
        Sink<T> globalSink = new Sink<>(resourceClass, name, name);
        globalSink.registerTo("$");
        Source<T> globalSource = new Source<>(resourceClass, name, resource);
        globalSource.registerTo("$");
        globalSinks.put(name, globalSink);
        globalSources.put(name, globalSource);
    }

    public void setLinkage(String source, String sink) {
        this.namedLinks.put(source, sink);
    }

    public void link() {
        HashMap<String, Sink<?>> sinkMap = new HashMap<>();
        HashMap<String, Source<?>> sourceMap = new HashMap<>();

        for (Sink<?> sink : globalSinks.values()) {
            sinkMap.put("$." + sink.getName(), sink);
        }
        for (Source<?> source : globalSources.values()) {
            sourceMap.put("$." + source.getName(), source);
        }

        for (Pass pass : passes.values()) {
            for (Sink<?> sink : pass.getSinks()) {
                sinkMap.put(pass.getName() + "." + sink.getName(), sink);
            }
            for (Source<?> source : pass.getSources()) {
                sourceMap.put(pass.getName() + "." + source.getName(), source);
            }
        }

        List<Pair<Source<?>, Sink<?>>> links = new ArrayList<>(namedLinks.size());
        for (String sourceName : namedLinks.keySet()) {
            Source<?> source = sourceMap.get(sourceName);
            Sink<?> sink = sinkMap.get(namedLinks.get(sourceName));
            links.add(new Pair<>(source, sink));
        }
        List<Pair<Source<?>, Sink<?>>> unprocessed_links = new ArrayList<>(links);

        while (!unprocessed_links.isEmpty()) {
            boolean processed_one = false;
            for (int i = 0; i < unprocessed_links.size(); i++) {
                Sink sink = unprocessed_links.get(i).getValue();
                Source source = unprocessed_links.get(i).getKey();
                if (!sink.getValueClass().equals(source.getValueClass()))
                    throw new AspectGraphLinkingException("the " + sink + " and " + source + " type to not match");
                if (source.get() != null) {
                    processed_one = true;
                    sink.link(sink.getValueClass().cast(source.get()));
                    String correlatedSourceName = sink.getLocation();
                    Source correlatedSource = sourceMap.get(correlatedSourceName);

                    if (correlatedSource == null)
                        throw new AspectGraphLinkingException(sink + " is correlated to an unknown source");
                    if (!correlatedSource.getValueClass().equals(sink.getValueClass()))
                        throw new AspectGraphLinkingException(sink + " is correlated to an illegal resource : wrong type");

                    correlatedSource.connect(correlatedSource.getValueClass().cast(sink.get()));

                    unprocessed_links.remove(i);
                    i--;
                }
            }
            if (!processed_one)
                throw new AspectGraphLinkingException("no pointers to resources found to connect the graph");
        }
        //error checking:
        //checking if the global resources are connected:
        for(Sink globalSink : globalSinks.values())
            if(globalSink.get() == null)
                throw new AspectGraphLinkingException("\nthe global " + globalSink + " is not connected to any source");
        for(Source globalSource : globalSources.values())
            if(globalSource.get() == null)
                throw new AspectGraphLinkingException("\nthe global " + globalSource + " has not pointer to its source");

        //checking if all sinks and sources are connected
        for (Pass pass : passes.values()) {
            for (Sink sink : pass.getSinks()) {
                if(sink.get() == null){
                    throw new AspectGraphLinkingException("\n" + sink + " of " + pass + "is not connected to any source");
                }
            }

            for (Source source : pass.getSources()) {
                if (source.get() == null) {
                    throw new AspectGraphLinkingException("\n" + source + " of " + pass + " has no pointer to its source");
                }
            }
        }
    }

    public void complete() {
        HashMap<String, String> namedLinksReversed = new HashMap<>();
        for (String source : namedLinks.keySet()) {
            namedLinksReversed.put(namedLinks.get(source), source);
        }

        HashSet<Pass> addedPasses = new HashSet<>();


        Pass pass = (Pass) passes.values().toArray()[0];
        passQueue.clear();
        constructPassQueue(passQueue, pass, namedLinksReversed, addedPasses);

        for(Pass toComplete : passQueue){
            toComplete.complete();
        }

    }

    public void constructPassQueue(ArrayList<Pass> queue, Pass activePass,
                                   HashMap<String, String> namedLinksReversed,
                                   HashSet<Pass> addedPasses) {

        final List<Pass> before = new ArrayList<>();
        final List<Pass> after = new ArrayList<>();
        for (Sink sink : activePass.getSinks()) {
            String connection = namedLinksReversed.get(activePass.getName() + "." + sink.getName());
            String connectedPassName = connection.split("\\.")[0];
            //connection is to a global resource
            if (connectedPassName.equals("$")) continue;
            Pass connectedPass = passes.get(connectedPassName);
            if (!addedPasses.contains(connectedPass))
                before.add(connectedPass);
        }
        for (Source source : activePass.getSources()) {
            String connection = namedLinks.get(activePass.getName() + "." + source.getName());
            if (connection == null) continue;
            String connectedPassName = connection.split("\\.")[0];
            //connection is to a global resource
            if (connectedPassName.equals("$")) continue;
            Pass connectedPass = passes.get(connectedPassName);
            if (!addedPasses.contains(connectedPass))
                after.add(connectedPass);
        }
        addedPasses.add(activePass);
        for (Pass pass : before) {
            constructPassQueue(queue, pass, namedLinksReversed, addedPasses);
        }
        queue.add(activePass);
        for (Pass pass : after) {
            constructPassQueue(queue, pass, namedLinksReversed, addedPasses);
        }
    }

    public void addJob(DrawingJob job, RenderQueuePass queuePass) {
        if (passes.containsKey(queuePass.getName())) {
            queuePass.addJob(job);
        }else{
            throw new AspectUnregisteredPassException(queuePass + " is not a registered for the graph : " + this);
        }
    }



    public void render() {
        //smarter pls!!
        for (Pass pass : passQueue) {
            pass.execute();
        }

        resetAllPasses();
    }

    private void resetAllPasses() {
        for (Pass pass : passes.values()) {
            pass.reset();
        }
    }

    public Pass getPass(String passName){
        return passes.get(passName);
    }

    public Renderer getRenderer(){
        return renderer;
    }

}
