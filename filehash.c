#include <stdlib.h>
#include <assert.h>

#include "fio.h"
#include "flist.h"
#include "hash.h"

#define HASH_BUCKETS	512
#define HASH_MASK	(HASH_BUCKETS - 1)

unsigned int file_hash_size = HASH_BUCKETS * sizeof(struct flist_head);

static struct flist_head *file_hash;
static struct fio_mutex *hash_lock;

static unsigned short hash(const char *name)
{
	return jhash(name, strlen(name), 0) & HASH_MASK;
}

void remove_file_hash(struct fio_file *f)
{
	fio_mutex_down(hash_lock);

	if (fio_file_hashed(f)) {
		assert(!flist_empty(&f->hash_list));
		flist_del_init(&f->hash_list);
		fio_file_clear_hashed(f);
	}

	fio_mutex_up(hash_lock);
}

static struct fio_file *__lookup_file_hash(const char *name)
{
	struct flist_head *bucket = &file_hash[hash(name)];
	struct flist_head *n;

	flist_for_each(n, bucket) {
		struct fio_file *f = flist_entry(n, struct fio_file, hash_list);

		if (!f->file_name)
			continue;

		if (!strcmp(f->file_name, name)) {
			assert(f->fd != -1);
			return f;
		}
	}

	return NULL;
}

struct fio_file *lookup_file_hash(const char *name)
{
	struct fio_file *f;

	fio_mutex_down(hash_lock);
	f = __lookup_file_hash(name);
	fio_mutex_up(hash_lock);
	return f;
}

struct fio_file *add_file_hash(struct fio_file *f)
{
	struct fio_file *alias;

	if (fio_file_hashed(f))
		return NULL;

	INIT_FLIST_HEAD(&f->hash_list);

	fio_mutex_down(hash_lock);

	alias = __lookup_file_hash(f->file_name);
	if (!alias) {
		fio_file_set_hashed(f);
		flist_add_tail(&f->hash_list, &file_hash[hash(f->file_name)]);
	}

	fio_mutex_up(hash_lock);
	return alias;
}

void file_hash_exit(void)
{
	unsigned int i, has_entries = 0;

	fio_mutex_down(hash_lock);
	for (i = 0; i < HASH_BUCKETS; i++)
		has_entries += !flist_empty(&file_hash[i]);
	fio_mutex_up(hash_lock);

	if (has_entries)
		log_err("fio: file hash not empty on exit\n");

	file_hash = NULL;
	fio_mutex_remove(hash_lock);
	hash_lock = NULL;
}

void file_hash_init(void *ptr)
{
	unsigned int i;

	file_hash = ptr;
	for (i = 0; i < HASH_BUCKETS; i++)
		INIT_FLIST_HEAD(&file_hash[i]);

	hash_lock = fio_mutex_init(1);
}
