#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "seg_2bitsmap.h"
#include "iohist_hash.h"

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

void
remove_seg(ul map2[], ul num, ull block)
{
    unsigned int idx, bit;
    idx = MAP2_IDX(block);
    bit = MAP2_BIT(block);
    assert(test_blk(map2, idx, bit, START));
    do {
        set_bit(map2, idx, bit, EMPTY);
        bit++;
        if (bit == BLKS_PER_MAP2) {
            idx++;
            bit=0;
        }
    }while (idx<num && test_blk(map2, idx, bit, IN));
    printf("<<<<<<<<<<REMOVE:%llu-%llu\n", block, (ull)MAP2_BLK(idx, bit)-1);
}

void
insert_seg(ul map2[], ull block, ul nr_blk, OverlapFunc overlap, void *ctx)
{
    unsigned int idx, bit;
    unsigned long finish=0;
    idx = MAP2_IDX(block);
    bit = MAP2_BIT(block);

    unsigned char blk;

    //pre start.
    blk = get_blk(map2, idx, bit);
    if (blk==EMPTY) {
    }else if(blk==START) {
        printf("START0<<<<<<block: %llu, start: %llu, len: %lu\n", block, block, nr_blk);
        overlap(block, block, nr_blk, ctx);
    }else if(blk==IN) {
        unsigned int prev_idx = idx;
        unsigned int prev_bit = bit;
        do {
            prev_bit += BLKS_PER_MAP2;
            prev_idx = prev_bit > BLKS_PER_MAP2 ? prev_idx : prev_idx-1;
            prev_bit = (prev_bit-1)%BLKS_PER_MAP2;
        }while (!test_blk(map2, prev_idx, prev_bit, START));
        printf("START1<<<<<<block: %llu, start: %llu, len: %lu\n", (ull)MAP2_BLK(prev_idx, prev_bit), block, nr_blk);
        overlap((ull)MAP2_BLK(prev_idx,prev_bit), block, nr_blk, ctx);
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
    while (finish<nr_blk) {
        if (test_blk(map2, idx, bit, START)) {
            printf("IN<<<<<<block: %llu(%u,%u), start: %llu, len: %lu\n", (ull)MAP2_BLK(idx,bit), idx, bit, block, nr_blk);
            overlap(MAP2_BLK(idx,bit), block, nr_blk, ctx);
        }
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
    printf("%p::", map2);
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
    printf("\n");
}

void
overlap_divide(ull block, ull start, ul len, void *ctx)
{
    struct fio_file *f = (struct fio_file *)ctx;
    struct thread_data *td = f->td;
    struct io_piece *ipo;
    struct iohist_key key;
    set_iohist_key(&key, f, block);
    ipo = iohist_hash_find(td, &key);
    assert(ipo);
    if (start<=block) {
        if (start+len < block + ipo->nr_blk) {
            // change nr_blk and block.
            ipo->nr_blk = block + ipo->nr_blk - (start + len);
            ipo->block = start + len;
            // deleted from hashtable.
            iohist_hash_del(td, ipo);
            // set 2bitsmap.
            set_bit(f->file_map2, MAP2_IDX(ipo->block), MAP2_BIT(ipo->block), START);
            // change hash, then add.
            set_iohist_key(&key, f, ipo->block);
            iohist_hash_add(td, ipo);
            printf("<<<<<<<<<<CHG:%p,%llu-%llu\n",ipo, ipo->block, ipo->block+ipo->nr_blk);
        }else {
            // deleted from hashtable.
            iohist_hash_del(td, ipo);
            // deleted from list.
            printf("<<<<<<<<<<DEL:%p,%llu-%llu\n",ipo, ipo->block, ipo->block+ipo->nr_blk);
            flist_del(&ipo->list);
            assert(ipo->flags & IP_F_ONLIST);
            ipo->flags &= ~IP_F_ONLIST;
            td->io_hist_len--;
            free(ipo);
        }
    }else {
        if (start+len < block + ipo->nr_blk) {
            // there is a new ipo at the tail.
            struct io_piece *tail_ipo = malloc(sizeof(struct io_piece));
            init_ipo(tail_ipo);
            tail_ipo->file = f;
            tail_ipo->block = start+len;
            tail_ipo->len = block + ipo->nr_blk - (start+len);
            td->unique_ops->copy(&tail_ipo->unique_version, &ipo->unique_version);
            // set 2bitsmap.
            set_bit(f->file_map2, MAP2_IDX(tail_ipo->block), MAP2_BIT(tail_ipo->block), START);
            // add tail_ipo to hashtable.
            set_iohist_key(&key, f, tail_ipo->block);
            iohist_hash_add(td, tail_ipo);
            // add tail_ipo to list.
            printf("<<<<<<<<<<ADD:%p, %llu-%llu\n",tail_ipo, tail_ipo->block, tail_ipo->block+tail_ipo->nr_blk);
            INIT_FLIST_HEAD(&tail_ipo->list);
            flist_add_tail(&tail_ipo->list, &td->io_hist_list);
            tail_ipo->flags |= IP_F_ONLIST;
            td->io_hist_len++;
        }
        // change origanal ipo's len.
        ipo->nr_blk = start - block;
        printf("<<<<<<<<<<CHG:%p,%llu-%llu\n",ipo, ipo->block, ipo->block+ipo->nr_blk);
    }
}

void
clear_map(ul map2[], void *ctx)
{
    struct fio_file *f = (struct fio_file *)ctx;
    struct thread_data *td = f->td;
    if (map2) {
        if (!(td && td->io_hist_len))
            memset(f->file_map2, 0, 2 * f->num_maps * sizeof(unsigned long));
    }
}
