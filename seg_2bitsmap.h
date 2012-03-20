#ifndef FIO_SEG_2BITS_MAP
#define FIO_SEG_2BITS_MAP

typedef unsigned long long ull;
typedef unsigned long ul;

#define BITS_PER_BLK    (2)
#define BLK_MASK    (~((~0)<<BITS_PER_BLK))
#define BLKS_PER_MAP2   (8 / BITS_PER_BLK * sizeof(unsigned long))
#define MAP2_IDX(b)  ((b) / BLKS_PER_MAP2)
#define MAP2_BIT(b)  ((b) & (BLKS_PER_MAP2 - 1))
#define MAP2_BLK(idx, bit)  ((idx)*BLKS_PER_MAP2+(bit))

#define EMPTY   (0x0)
#define START   (0x2)
#define IN      (0x3)
#define ERR     (0x1)    


typedef void (*OverlapFunc)(ull block, ull start, ul len, void *ctx); 

void remove_seg(ul map2[], ul num, ull block);
void insert_seg(ul map2[], ull block, ul nr_blk, OverlapFunc overlap, void *ctx);
void print_map(unsigned long map2[], unsigned long nmaps);

/* overlap handlers */
void overlap_divide(ull block, ull start, ul len, void *ctx);
void clear_map(ul map2[], void *ctx);

#endif /* FIO_SEG_2BITS_MAP */
