#ifndef FIO_IOHIST_HASH_H
#define FIO_IOHIST_HASH_H

#include "fio.h"

struct iohist_key {
    struct fio_file *file;
	unsigned long block;
};

int iohist_hash_init(struct thread_data *td);
void iohist_hash_exit(struct thread_data *td);
struct io_piece *iohist_hash_find(struct thread_data *td, const struct iohist_key *pkey);
struct io_piece *iohist_hash_add(struct thread_data *, struct io_piece *);
void iohist_hash_del(struct thread_data *, struct io_piece *);

static inline void set_iohist_key(struct iohist_key *pkey, struct fio_file *file, unsigned long block)
{
    pkey->file = file;
    pkey->block = block;
}
static inline void io_piece_set_hashed(struct io_piece *ipo)
{
	(ipo)->flags |= IP_F_HASHED;
}
static inline void io_piece_clear_hashed(struct io_piece *ipo)
{
	(ipo)->flags &= ~IP_F_HASHED;
}
static inline int io_piece_hashed(struct io_piece *ipo)
{
	return ((ipo)->flags & IP_F_HASHED) != 0;
}

#endif
