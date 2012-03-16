#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BITS_PER_BLK    (2)
#define BLKS_PER_MAP2   (8 / BITS_PER_BLK * sizeof(unsigned long))
#define MAP2_IDX(b)  ((b) / BLKS_PER_MAP2)
#define MAP2_BIT(b)  ((b) & (BLKS_PER_MAP2 - 1))

#define blocks   (4 * BLKS_PER_MAP2)
#define nmaps   (blocks/BLKS_PER_MAP2)

#define maxlen  (2)

#define EMPTY   (0x0)
#define START   (0x2)
#define IN      (0x3)
#define END     (0x1)    

unsigned long map2[nmaps];

unsigned char
get_blk(unsigned int idx, unsigned int bit)
{
    unsigned long long mask0, mask;
    mask = IN << (bit*2);
    return ((map2[idx] & mask) >> (bit*2))&0xFF;
}

void
set_bit(unsigned int idx, unsigned int bit, unsigned char stat)
{
    unsigned long long mask0, mask;
    mask0 = 0x3 << (bit*2);
    mask = stat << (bit*2);
    map2[idx] &= ~mask0;
    map2[idx] |= mask;
}

void
insert_seg(unsigned long long offset, unsigned long len)
{
    int i,j;
    unsigned int idx, bit;
    unsigned long finish=0;
    idx = MAP2_IDX(offset);
    bit = MAP2_BIT(offset);

    unsigned char blk;
    unsigned long long mask0, mask;

    printf("%lu,%lu\n", idx, bit);
    //pre start.
    blk = get_blk(idx, bit);
    if (blk==EMPTY) {
    }else if(blk==START) {
        //the first overwrite.
        //if len>this.len remove this.
        //else this.offset += len
        //      this.len -= len
        printf("idx=%u,bit=%u\n", idx, bit);
    }else if(blk==IN|blk==END) {
        //change pre bit stat.
        unsigned int prev_idx,prev_bit;
        if (bit==0) {
            prev_bit = BLKS_PER_MAP2-1;
            prev_idx = idx-1;
        }else {
            prev_bit = bit-1;
            prev_idx = idx;
        }
        blk = get_blk(prev_idx, prev_bit);
        if (blk==START) {
        }else if (blk==IN) {
            //change prev bit stat IN to END.
            set_bit(prev_idx, prev_bit, END);
        }
    }
    //start.
    set_bit(idx, bit, START);
    finish++;
    bit++;
    if (bit == BLKS_PER_MAP2) {
        idx++;
        bit=0;
    }
    //in.
    while (finish<len-1) {
        set_bit(idx, bit, IN);
        finish++;
        bit++;
        if (bit == BLKS_PER_MAP2) {
            idx++;
            bit=0;
        }
    }
    //end.
    if (finish < len) {
        set_bit(idx, bit, END);
        finish++;
        bit++;
        if (bit == BLKS_PER_MAP2) {
            idx++;
            bit=0;
        }
    }
    //post end.
    //change post bit stat.
    unsigned int next_idx,next_bit;
    next_bit = bit;
    next_idx = idx;
    if (next_idx <= nmaps) {
        blk = get_blk(next_idx, next_bit);
        if (blk==EMPTY) {
        }else if (blk==START) {
        }else if (blk==IN|blk==END) {
            //change post bit stat IN or END to START.
            set_bit(next_idx, next_bit, START);
        }
    }
}

void
generate_seg(unsigned long long *poffset, unsigned long *plen)
{
    unsigned long long offset;
    unsigned long len;

    offset = rand()%blocks;
    len = rand()%maxlen+1;
    if ((offset + len) > blocks)
        len = blocks - offset;

    *poffset = offset;
    *plen = len;
}

void
print_map(void)
{
    int i,j;
    unsigned char blk;
    unsigned long long mask;
    for (i=0; i<nmaps; i++) {
        mask = 0x3;
        for (j=0; j<BLKS_PER_MAP2; j++) {
            blk = ((map2[i] & mask) >> (j*2))&0xFF;
            if (blk==EMPTY) {
                //printf("%02d",j);
                printf("-+");
                //printf("=+");
            }else if(blk==START) {
                printf("*#");
            }else if(blk==IN) {
                printf("##");
            }else if(blk==END) {
                printf("#*");
            }
            mask <<= 2;
        }
        printf("|");
    }
}

int
main(int argc, char *argv[])
{
    int i;
    unsigned long long size = 0;
    srand(time(NULL));

    //while (size < blocks<<4) {
    while (1) {
        unsigned long long offset;
        unsigned long len;
        generate_seg(&offset, &len);
        printf("%llu,%lu\n", offset, len);
        insert_seg(offset, len);
        getchar();
        print_map();
        printf("\n");
        size += len;
    }
    return 0;
}

