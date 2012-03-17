#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "seg_2bitsmap.h"

static inline unsigned char
get_blk(unsigned long map2[], unsigned int idx, unsigned int bit)
{
    return ((map2[idx]) >> (bit*BITS_PER_BLK))&BLK_MASK;
}

static inline int
test_blk(ul map2[], unsigned int idx, unsigned bit, unsigned char stat)
{
    return (((map2[idx]) >> (bit*BITS_PER_BLK))&BLK_MASK) == stat;
}


static inline void
set_bit(unsigned long map2[], unsigned int idx, unsigned int bit, unsigned char stat)
{
    map2[idx] &= ~(BLK_MASK << (bit*BITS_PER_BLK));
    map2[idx] |= stat << (bit*BITS_PER_BLK);
}

int
remove_seg(ul offset)
{
    return 0;
}

void
insert_seg(ul map2[], ull offset, ul len, OverlapFunc overlap, void *ctx)
{
    unsigned int idx, bit;
    unsigned long finish=0;
    idx = MAP2_IDX(offset);
    bit = MAP2_BIT(offset);

    unsigned char blk;

    //pre start.
    blk = get_blk(map2, idx, bit);
    if (blk==EMPTY) {
    }else if(blk==START) {
        overlap(offset, offset, len, ctx);
    }else if(blk==IN) {
        unsigned int prev_idx = idx;
        unsigned int prev_bit = bit;
        do {
            prev_bit += BLKS_PER_MAP2;
            prev_idx = prev_bit > BLKS_PER_MAP2 ? prev_idx : prev_idx-1;
            prev_bit = (prev_bit-1)%BLKS_PER_MAP2;
        }while (test_blk(map2, prev_idx, prev_bit, START));
        overlap(MAP2_BLK(prev_idx,prev_bit), offset, len, ctx);
    }else if(blk==ERR) {
        //2bits-map error.
    }
    //start.
    set_bit(map2, idx, bit, START);
    finish++;
    bit++;
    if (bit == BLKS_PER_MAP2) {
        idx++;
        bit=0;
    }
    //in.
    while (finish<len) {
        if (test_blk(map2, idx, bit, START))
            overlap(MAP2_BLK(idx,bit), offset, len, ctx);
        set_bit(map2, idx, bit, IN);
        finish++;
        bit++;
        if (bit == BLKS_PER_MAP2) {
            idx++;
            bit=0;
        }
    }
}

void
print_map(unsigned long map2[], unsigned long nmaps)
{
    int idx,bit;
    unsigned char blk;
    for (idx=0; idx<nmaps; idx++) {
        for (bit=0; bit<BLKS_PER_MAP2; bit++) {
            blk = get_blk(map2, idx, bit);
            if (blk==EMPTY) {
                //printf("%02d",j);
                printf("-+");
                //printf("=+");
            }else if(blk==START) {
                printf("*#");
            }else if(blk==IN) {
                printf("##");
            }else if(blk==ERR) {
                printf("$$");
            }
        }
        printf("|");
    }
}
