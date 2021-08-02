package org.strobe.gfx.opengl.bindables.vao;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;

import java.util.*;

import static org.lwjgl.opengl.GL30.*;

@Pool(VaoPool.class)
public class Vao extends Bindable<VaoPool> {

    private final int ID;

    private final Vbo[] vbos;
    private final int[] attributeLocations;
    private final int attribCount;
    private final int vboCount;

    public Vao(Graphics gfx, int vertexCount, Ibo ibo, String... formats) {
        super(gfx);
        pool.add(this);

        this.attribCount = formats.length;
        int[] locations = new int[formats.length];
        int[] strides = new int[formats.length];
        int[] types = new int[formats.length];
        int[] divisors = new int[formats.length];
        int[] usages = new int[formats.length];
        int[] coverages = new int[formats.length];

        for (int j = 0; j < formats.length; j++) {
            String[] spaceSplit = formats[j].trim().split(" ");
            if (spaceSplit.length != 3) throw new IllegalArgumentException("vao format must match layout(...) in type");
            //parse layout
            String layout = spaceSplit[0];
            int divisor = -1;
            int location = -1;
            int usage = -1;
            String[] layoutSplit = layout.split("\\(");
            if (layoutSplit.length != 2) throw new IllegalArgumentException("vao format doesn't have valid brackets");
            String layoutKeyword = layoutSplit[0];
            if (!layoutKeyword.equals("layout"))
                throw new IllegalArgumentException("vao format doesn't start with layout");
            if (!layoutSplit[1].endsWith(")"))
                throw new IllegalArgumentException("vao format doesn't have valid brackets");
            String[] layoutArgs = layoutSplit[1].substring(0, layoutSplit[1].length() - 1).split(",");
            for (int i = 0; i < layoutArgs.length; i++) {
                String[] assignmentSplit = layoutArgs[i].trim().split("=");
                if (assignmentSplit.length != 2)
                    throw new IllegalArgumentException("vao format has an invalid assignment");
                String attrib = assignmentSplit[0].trim();
                String value = assignmentSplit[1].trim();
                if (attrib.equals("location")) {
                    try {
                        location = Integer.parseInt(value);
                    } catch (NumberFormatException e) {
                        throw new IllegalArgumentException("vao format contains an invalid location");
                    }
                } else if (attrib.equals("divisor")) {
                    try {
                        divisor = Integer.parseInt(value);
                    } catch (NumberFormatException e) {
                        throw new IllegalArgumentException("vao format contains an invalid divisor");
                    }
                } else if (attrib.equals("usage")) {
                    if (value.equals("static")) {
                        usage = GL_STATIC_DRAW;
                    } else if (value.equals("dynamic")) {
                        usage = GL_DYNAMIC_DRAW;
                    } else {
                        throw new IllegalArgumentException("vao format contains an invalid usage");
                    }
                } else {
                    throw new IllegalArgumentException("vao format contains an invalid layout argument");
                }
            }

            if (location == -1) throw new IllegalArgumentException("vao format doesn't contain a location");
            if (divisor == -1) divisor = 0;
            if (usage == -1) usage = GL_STATIC_DRAW;
            locations[j] = location;
            divisors[j] = divisor;
            usages[j] = usage;

            String inKeyword = spaceSplit[1];
            if (!inKeyword.equals("in"))
                throw new IllegalArgumentException("vao format doesn't contain the in keyword");

            //parse type
            String typeStr = spaceSplit[2];
            int type;
            int stride;
            int coverage;
            switch (typeStr) {
                case "vec2":
                    type = GL_FLOAT;
                    stride = 2;
                    coverage = 1;
                    break;
                case "vec4":
                    type = GL_FLOAT;
                    stride = 4;
                    coverage = 1;
                    break;
                case "vec3":
                    type = GL_FLOAT;
                    stride = 3;
                    coverage = 1;
                    break;
                case "mat4":
                    type = GL_FLOAT;
                    stride = 4;
                    coverage = 4;
                default:
                    throw new IllegalStateException("vao format type is invalid");
            }
            types[j] = type;
            strides[j] = stride;
            coverages[j] = coverage;
        }

        //check for duplicated locations
        //TODO take coverage into account
        Set<Integer> locSet = new HashSet<>();
        for (int loc : locations) locSet.add(loc);
        if (locSet.size() != locations.length)
            throw new IllegalStateException("vao format contains duplicated locations");

        //save all locations
        ArrayList<Integer> attribLocs = new ArrayList<>();
        for (int i = 0; i < locations.length; i++) {
            for (int j = 0; j < coverages[i]; j++) {
                attribLocs.add(locations[i] + j);
            }
        }
        this.attributeLocations = new int[attribLocs.size()];
        for (int i = 0; i < attributeLocations.length; i++) attributeLocations[i] = attribLocs.get(i);


        //get max location
        int maxLoc = Integer.MIN_VALUE;
        for (int loc : locations) maxLoc = Math.max(maxLoc, loc);
        vbos = new Vbo[maxLoc + 1];


        VboBuilder merger = new VboBuilder(locations, types, strides, divisors, coverages, usages);
        vboCount = merger.getVboCount();

        ID = glGenVertexArrays();
        gfx.bind(this);
        ibo.connectToBoundVao(gfx);

        for (Vao.VboData vboData : merger) {
            Vbo vbo = new Vbo(gfx, vertexCount, vboData.locations, vboData.strides,
                    vboData.types, vboData.divisors, vboData.coverages, vboData.usage);
            for (int loc : vboData.locations) {
                vbos[loc] = vbo;
            }
            vbo.connectToBoundVao(gfx);
        }
        gfx.unbind(this);

        pool.add(this);
    }

    public Vbo getVboForLocation(int location) {
        Vbo vbo = vbos[location];
        if (vbo == null) throw new IllegalArgumentException("vao doesn't have a attribute at location :" + location);
        return vbo;
    }

    public void bufferLocation(Graphics gfx, int location, float[] data) {
        getVboForLocation(location).bufferLocation(gfx, location, data);
    }

    @Override
    protected void bind(Graphics gfx) {
        glBindVertexArray(ID);
        enableVertexAttribArrays();
    }

    private void enableVertexAttribArrays() {
        for (int i = 0; i < attributeLocations.length; i++) glEnableVertexAttribArray(attributeLocations[i]);
    }

    @Override
    protected void unbind(Graphics gfx) {
        glBindVertexArray(0);
        disableVertexAttribArrays();
    }

    private void disableVertexAttribArrays() {
        for (int i = 0; i < attributeLocations.length; i++) glDisableVertexAttribArray(attributeLocations[i]);
    }

    @Override
    protected void dispose(Graphics gfx) {
        glDeleteVertexArrays(ID);
    }

    private static class VboBuilder implements Iterable<Vao.VboData> {

        private final ArrayList<VboData> vbos = new ArrayList<>();

        public VboBuilder(int[] locations, int[] types, int[] strides,
                          int[] divisors, int[] coverages, int[] usages) {
            ArrayList<Integer> staticAttributes = new ArrayList<>();
            ArrayList<Integer> dynamicAttributes = new ArrayList<>();

            for (int i = 0; i < locations.length; i++) {
                if (usages[i] == GL_STATIC_DRAW) {
                    staticAttributes.add(i);
                } else if (usages[i] == GL_DYNAMIC_DRAW) {
                    dynamicAttributes.add(i);
                } else throw new IllegalStateException("not a valid usage for " + getClass().getSimpleName());
            }

            //create merged attributes for all static attributes
            int[] staticLocations = new int[staticAttributes.size()];
            int[] staticTypes = new int[staticAttributes.size()];
            int[] staticStrides = new int[staticAttributes.size()];
            int[] staticDivisors = new int[staticAttributes.size()];
            int[] staticCoverages = new int[staticAttributes.size()];
            for (int i = 0; i < staticAttributes.size(); i++) {
                int j = staticAttributes.get(i);
                staticLocations[i] = locations[j];
                staticTypes[i] = types[j];
                staticStrides[i] = strides[j];
                staticDivisors[i] = divisors[j];
                staticCoverages[i] = coverages[j];
            }

            vbos.add(new VboData(staticLocations, staticTypes, staticStrides,
                    staticDivisors, staticCoverages, GL_STATIC_DRAW));

            //create unique vbos for each attribute!
            for (int i : dynamicAttributes) {
                vbos.add(new VboData(new int[]{locations[i]},
                        new int[]{types[i]}, new int[]{strides[i]}, new int[]{divisors[i]}, new int[]{coverages[i]}, GL_DYNAMIC_DRAW));
            }
        }

        public int getVboCount() {
            return vbos.size();
        }

        @Override
        public Iterator<Vao.VboData> iterator() {
            return vbos.iterator();
        }
    }

    private static class VboData {
        private int[] locations;
        private int[] types;
        private int[] strides;
        private int[] divisors;
        private int[] coverages;
        private int usage;

        public VboData(int[] locations, int[] types, int[] strides, int[] divisors, int[] coverages, int usages) {
            this.locations = locations;
            this.types = types;
            this.strides = strides;
            this.divisors = divisors;
            this.coverages = coverages;
            this.usage = usages;
        }
    }
}
