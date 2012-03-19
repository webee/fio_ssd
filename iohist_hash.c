#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "fio.h"
#include "flist.h"
#include "hash.h"
#include "iohist_hash.h"

#define HASH_BUCKETS	(1024*1024)
#define HASH_MASK	(HASH_BUCKETS - 1)

//unsigned int iohist_hash_size = HASH_BUCKETS * sizeof(struct flist_head);

static unsigned short hash(const struct iohist_key *pkey)
{
	return jhash((void *)pkey, sizeof(struct iohist_key), 0) & HASH_MASK;
}

void iohist_hash_del(struct thread_data *td, struct io_piece *ipo)
{
	if (io_piece_hashed(ipo)) {
		assert(!flist_empty(&ipo->hash_list));
		flist_del_init(&ipo->hash_list);
		io_piece_clear_hashed(ipo);
	}
}

static struct io_piece *__iohist_hash_find(struct thread_data *td, const struct iohist_key *pkey)
{
    struct flist_head *iohist_hash=td->iohist_hash;
	struct flist_head *bucket = &iohist_hash[hash(pkey)];
	struct flist_head *n;

	flist_for_each(n, bucket) {
        struct iohist_key _key;
		struct io_piece *ipo = flist_entry(n, struct io_piece, hash_list);
        set_iohist_key(&_key, ipo->file, ipo->block);

		if (!memcmp(&_key, pkey, sizeof(struct iohist_key)))
			return ipo;
	}

	return NULL;
}

struct io_piece *iohist_hash_find(struct thread_data *td, const struct iohist_key *pkey)
{
    //struct flist_head *iohist_hash=td->iohist_hash;
	struct io_piece *ipo;

	ipo = __iohist_hash_find(td, pkey);
	return ipo;
}

struct io_piece *iohist_hash_add(struct thread_data *td, struct io_piece *ipo)
{
    struct flist_head *iohist_hash=td->iohist_hash;
	struct io_piece *_ipo;
    struct iohist_key key;

	if (io_piece_hashed(ipo))
		return NULL;

    set_iohist_key(&key, ipo->file, ipo->block);
	INIT_FLIST_HEAD(&ipo->hash_list);

	_ipo = __iohist_hash_find(td, &key);
	if (!_ipo) {
		io_piece_set_hashed(ipo);
		flist_add_tail(&ipo->hash_list, &iohist_hash[hash(&key)]);
	}

	return _ipo;
}

void iohist_hash_exit(struct thread_data *td)
{
    struct flist_head *iohist_hash=td->iohist_hash;
	unsigned int i, has_entries = 0;

	for (i = 0; i < HASH_BUCKETS; i++)
		has_entries += !flist_empty(&iohist_hash[i]);

	if (has_entries)
		log_err("fio: file hash not empty on exit\n");

    free(iohist_hash);
	iohist_hash = NULL;
}

int iohist_hash_init(struct thread_data *td)
{
    struct flist_head *iohist_hash;
	unsigned int i;

    if (!((td->o.randomagain || td->o.norandommap))&&
                (td->o.min_bs[DDIR_WRITE] != td->o.max_bs[DDIR_WRITE])) {
        return 0;
    }
	td->iohist_hash = malloc(HASH_BUCKETS*sizeof(struct flist_head));
    if (!td->iohist_hash)
        return 1;
    iohist_hash=td->iohist_hash;
	for (i = 0; i < HASH_BUCKETS; i++)
		INIT_FLIST_HEAD(&iohist_hash[i]);
    return 0;
}
