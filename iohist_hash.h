#ifndef FIO_FILE_HASH_H
#define FIO_FILE_HASH_H

struct iohist_key {
    struct fio_file *file;
	unsigned long long offset;
};

void iohist_hash_init(struct thread_data *td);
void iohist_hash_exit(struct thread_data *td);
struct io_piece *lookup_iohist_hash(struct thread_data *td, const struct iohist_key *pkey);
struct io_piece *add_iohist_hash(struct thread_data *, struct io_piece *);
void remove_iohist_hash(struct thread_data *, struct io_piece *);

static inline void set_iohist_key(struct io_piece *ipo, struct iohist_key *pkey)
{
    pkey->file = ipo->file;
    pkey->offset = ipo->offset;
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
