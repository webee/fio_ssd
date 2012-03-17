#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "fio.h"
#include "flist.h"
#include "hash.h"

#define HASH_BUCKETS	(1024*1024)
#define HASH_MASK	(HASH_BUCKETS - 1)

//unsigned int iohist_hash_size = HASH_BUCKETS * sizeof(struct flist_head);

static struct flist_head *iohist_hash;
static struct fio_mutex *hash_lock;

static unsigned short hash(struct iohist_key *pkey)
{
	return jhash(pkey, sizeof(struct iohist_key), 0) & HASH_MASK;
}

void remove_iohist_hash(struct io_piece *ipo)
{
	fio_mutex_down(hash_lock);

	if (io_piece_hashed(ipo)) {
		assert(!flist_empty(&ipo->hash_list));
		flist_del_init(&ipo->hash_list);
		io_piece_clear_hashed(ipo);
	}

	fio_mutex_up(hash_lock);
}

static struct io_piece *__lookup_iohist_hash(const struct iohist_key *pkey)
{
	struct flist_head *bucket = &iohist_hash[hash(pkey)];
	struct flist_head *n;

	flist_for_each(n, bucket) {
        struct iohist_key _key;
		struct io_piece *ipo = flist_entry(n, struct io_piece, hash_list);
        set_iohist_key(ipo, &_key);

		if (!memcmp(&_key, pkey))
			return ipo;
	}

	return NULL;
}

struct io_piece *lookup_iohist_hash(const struct iohist_key *pkey)
{
	struct io_piece *ipo;

	fio_mutex_down(hash_lock);
	ipo = __lookup_iohist_hash(pkey);
	fio_mutex_up(hash_lock);
	return ipo;
}

struct io_piece *add_iohist_hash(struct io_piece *ipo)
{
	struct io_piece *_ipo;
    struct iohist_key key;

	if (io_piece_hashed(ipo))
		return NULL;

    set_iohist_key(ipo, &key);
	INIT_FLIST_HEAD(&ipo->hash_list);

	fio_mutex_down(hash_lock);

	_ipo = __lookup_iohist_hash(&key);
	if (!_ipo) {
		io_piece_set_hashed(ipo);
		flist_add_tail(&ipo->hash_list, &iohist_hash[hash(&key)]);
	}

	fio_mutex_up(hash_lock);
	return _ipo;
}

void iohist_hash_exit(void)
{
	unsigned int i, has_entries = 0;

	fio_mutex_down(hash_lock);
	for (i = 0; i < HASH_BUCKETS; i++)
		has_entries += !flist_empty(&iohist_hash[i]);
	fio_mutex_up(hash_lock);

	if (has_entries)
		log_err("fio: file hash not empty on exit\n");

    free(iohist_hash);
	iohist_hash = NULL;
	fio_mutex_remove(hash_lock);
	hash_lock = NULL;
}

void iohist_hash_init(void)
{
	unsigned int i;

	iohist_hash = malloc(HASH_BUCKETS*sizeof(struct flist_head));
	for (i = 0; i < HASH_BUCKETS; i++)
		INIT_FLIST_HEAD(&iohist_hash[i]);

	hash_lock = fio_mutex_init(1);
}
