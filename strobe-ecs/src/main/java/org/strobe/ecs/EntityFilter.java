package org.strobe.ecs;

import javax.print.attribute.HashPrintServiceAttributeSet;
import java.util.*;

public abstract class EntityFilter {

    public static EntityFilter requireAll(EntityFilter... filters) {
        return new FilterAndFilter(filters);
    }

    public static EntityFilter requireAll(Class<? extends Component>... componentClasses) {
        return new ComponentAndFilter(componentClasses);
    }

    public static EntityFilter requireOne(EntityFilter... filters) {
        return new FilterOrFilter(filters);
    }

    public static EntityFilter requireOne(Class<? extends Component>... componentClasses) {
        return new ComponentOrFilter(componentClasses);
    }

    public static EntityFilter prohibitAll(EntityFilter... filters) {
        return new FilterNorFilter(filters);
    }

    public static EntityFilter prohibitAll(Class<? extends Component>... componentClasses) {
        return new ComponentNorFilter(componentClasses);
    }

    public static EntityFilter invert(EntityFilter filter) {
        return new NotFilter(filter);
    }

    /**
     * a set that contains all component indices the filter is sensitive to.
     */
    protected final Set<Integer> sensitivity = new HashSet<>();

    public abstract boolean eval(Entity entity);

    public boolean isSensitive(Class<? extends Component> componentClass){
        return isSensitive(ComponentType.getFor(componentClass));
    }

    protected boolean isSensitive(ComponentType componentType){
        return sensitivity.contains(componentType.getIndex());
    }

    private static abstract class ComponentFilter extends EntityFilter{

        protected final ComponentType[] componentTypes;

        protected ComponentFilter(Class<? extends Component>...componentClasses){
            Set<ComponentType> typeSet = new HashSet<>();
            for(int i=0;i< componentClasses.length;i++){
                typeSet.add(ComponentType.getFor(componentClasses[i]));
            }
            componentTypes = new ComponentType[typeSet.size()];
            int i=0;
            for(ComponentType type : typeSet){
                componentTypes[i++] = type;
                sensitivity.add(type.getIndex());
            }
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            ComponentAndFilter that = (ComponentAndFilter) o;
            if(componentTypes.length != that.componentTypes.length)return false;
            for(ComponentType componentType : componentTypes){
                boolean contains = false;
                for(ComponentType innerType : that.componentTypes){
                    if(componentType.equals(innerType)){
                        contains = true;
                        break;
                    }
                }
                if(!contains)return false;
            }
            return true;
        }

        @Override
        public int hashCode() {
            return Objects.hash(getClass());
        }
    }

    private static abstract class FilterFilter extends EntityFilter{

        protected final EntityFilter[] filters;

        protected FilterFilter(EntityFilter[] filters) {
            Set<EntityFilter> set = new HashSet<>();
            for(EntityFilter filter : filters){
                set.add(filter);
            }
            this.filters = new EntityFilter[set.size()];
            int i=0;
            for(EntityFilter filter : set){
                this.filters[i++] = filter;
                sensitivity.addAll(filter.sensitivity);
            }
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            FilterFilter that = (FilterFilter) o;
            if(filters.length != that.filters.length)return false;
            for(EntityFilter filter : filters){
                boolean contains = false;
                for(EntityFilter innerFilter : filters){
                    if(filter.equals(innerFilter)){
                        contains = true;
                        break;
                    }
                }
                if(!contains)return false;
            }
            return true;
        }

        @Override
        public int hashCode() {
            return Objects.hash(getClass());
        }
    }

    private static final class ComponentAndFilter extends ComponentFilter {

        public ComponentAndFilter(Class<? extends Component>[] componentClasses) {
            super(componentClasses);
        }

        @Override
        public boolean eval(Entity entity) {
            for (ComponentType componentType : componentTypes) {
                if (!entity.has(componentType)) return false;
            }
            return true;
        }

    }

    private static final class FilterAndFilter extends FilterFilter {

        public FilterAndFilter(EntityFilter[] filters) {
            super(filters);
        }

        @Override
        public boolean eval(Entity entity) {
            for (EntityFilter filter : filters) {
                if (!filter.eval(entity)) return false;
            }
            return true;
        }
    }

    private static final class ComponentOrFilter extends ComponentFilter{

        public ComponentOrFilter(Class<? extends Component>[] componentClasses) {
            super(componentClasses);
        }

        @Override
        public boolean eval(Entity entity) {
            for(ComponentType componentType : componentTypes){
                if(entity.has(componentType))return true;
            }
            return false;
        }
    }

    private static final class FilterOrFilter extends FilterFilter{

        public FilterOrFilter(EntityFilter[] filters) {
            super(filters);
        }

        @Override
        public boolean eval(Entity entity) {
            for(EntityFilter filter : filters){
                if(filter.eval(entity))return true;
            }
            return false;
        }
    }

    private static final class ComponentNorFilter extends ComponentFilter{

        public ComponentNorFilter(Class<? extends Component>... componentClasses) {
            super(componentClasses);
        }

        @Override
        public boolean eval(Entity entity) {
            for(ComponentType componentType : componentTypes){
                if(entity.has(componentType))return false;
            }
            return true;
        }
    }

    private static final class FilterNorFilter extends FilterFilter{

        protected FilterNorFilter(EntityFilter[] filters) {
            super(filters);
        }

        @Override
        public boolean eval(Entity entity) {
            for(EntityFilter entityFilter : filters){
                if(entityFilter.eval(entity))return false;
            }
            return true;
        }
    }

    private static final class NotFilter extends EntityFilter{

        private EntityFilter filter;

        protected NotFilter(EntityFilter filter){
            this.filter = filter;
            sensitivity.addAll(filter.sensitivity);
        }

        @Override
        public boolean eval(Entity entity) {
            return !filter.eval(entity);
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            NotFilter that = (NotFilter) o;
            return Objects.equals(filter, that.filter);
        }

        @Override
        public int hashCode() {
            return Objects.hash(filter, getClass());
        }
    }
}
