package org.marshal.intern;


/**
 * A bit array where every index can be set individually, implemented in a
 * way that allows bitwise operation, which makes comparing faster.
 */
public final class ComponentMask {

    private long[] mask = {0l};

    public void set(int index, boolean state){
        int mask_index = Math.floorDiv(index, Long.BYTES*8);
        int bit_index = index % (Long.BYTES*8);
        set(mask_index, bit_index, state);
    }

    private void set(int mask_index, int bit_index, boolean state){
        if(state){
            mask[mask_index] |= (1l<<bit_index);
        }else{
            mask[mask_index] &= ~(1l<<bit_index);
        }
    }

    public boolean get(int index){
        int mask_index = Math.floorDiv(index, Long.BYTES*8);
        int bit_index = index % (Long.BYTES*8);
        return get(mask_index, bit_index);
    }

    private boolean get(int mask_index, int bit_index){
        return (mask[mask_index] & (1l<<bit_index))!=0;
    }

    public boolean hasOneOf(ComponentMask mask){
        for(int i=0;i<Math.min(mask.mask.length, this.mask.length);i++){
            long collision = mask.mask[i] & this.mask[i];
            if(collision != 0)return true;
        }
        return false;
    }

    public void combineWith(ComponentMask mask) {
        for(int i=0;i<Math.min(mask.mask.length, this.mask.length);i++){
            this.mask[i] |= mask.mask[i];
        }
    }

    public String toString(){
        StringBuilder builder = new StringBuilder("{");
        for(int i=0;i<mask.length;i++){
            for(int j=0;j<Long.BYTES*8;j++){
                if(get(i,j))builder.append("1");
                else builder.append("0");
            }
        }
        builder.append("}");
        return builder.toString();
    }


}
