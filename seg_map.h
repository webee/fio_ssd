#ifndef FIO_SEG_MAP
#define FIO_SEG_MAP

#define BITS_PER_BLK    (2)
#define BLKS_PER_MAP2   (8 / BITS_PER_BLK * sizeof(unsigned long))
#define MAP2_IDX(b)  ((b) / BLKS_PER_MAP2)
#define MAP2_BIT(b)  ((b) & (BLKS_PER_MAP2 - 1))

#endif /* FIO_SEG_MAP */
