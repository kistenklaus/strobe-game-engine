package org.strobe.gfx;

import java.util.HashMap;
import java.util.Iterator;
import java.util.function.Function;
import java.util.function.Supplier;

public abstract class HashPool<T extends Bindable> extends BindablePool<T> {

    private final HashMap<Entry, T> pool = new HashMap<>();

    public HashPool(BindingMethod method) {
        super(method);
    }

    /**
     * registers a bindable into the HashPool if the bindable doesn't exist in the HashPool
     *
     * @param bindable the bindable to register
     * @param hash     the hash function to use for the map lookup with which to check if the bindable exists in the hashPool
     * @param equals   the equals function to use for the map lookup
     * @return a existing bindable that matches the hash and equals function. If there is no such bindable in the
     * hashPool it will return null.
     */
    public final T register(T bindable, Supplier<Integer> hash, Function<T, Boolean> equals) {
        Entry key = new Entry(bindable, hash, equals);
        if (pool.containsKey(key)) {
            return pool.get(key);
        } else {
            pool.put(key, bindable);
            return null;
        }
    }

    @Override
    public void dispose(Graphics gfx, T bindable) {

    }

    public Iterator<T> iterator() {
        return pool.values().iterator();
    }

    private final class Entry {
        private final T value;
        private final Supplier<Integer> hash;
        private final Function<T, Boolean> equals;

        public Entry(T value, Supplier<Integer> hash, Function<T, Boolean> equals) {
            this.value = value;
            this.hash = hash;
            this.equals = equals;
        }

        public int hashCode() {
            return hash.get();
        }

        public boolean equals(Object obj) {
            if (!(obj instanceof HashPool.Entry)) return false;
            Entry entry = (Entry) obj;
            return equals.apply(entry.value);
        }
    }
}
