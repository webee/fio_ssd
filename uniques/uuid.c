/*
 *  uuid based unique.
 */
#include "../fio.h"

static void
fio_uuid_set(struct fio_unique *u)
{
    uuid_generate(u->uuid);
}

static void
fio_uuid_copy(struct fio_unique *udst, struct fio_unique *usrc)
{
    uuid_copy(udst->uuid, usrc->uuid);
}

static char *
fio_uuid_to_string(struct fio_unique *u, char *s)
{
    uuid_unparse_lower(u->uuid, s);
    return s;
}

static int
fio_uuid_is_match(struct fio_unique *u1, struct fio_unique *u2)
{
    return !uuid_compare(u1->uuid, u2->uuid);
}

static struct unique_ops unique = {
    .name       = "uuid",
    .set        = fio_uuid_set,
    .copy       = fio_uuid_copy,
    .to_string  = fio_uuid_to_string,
    .is_match   = fio_uuid_is_match,
};

static void fio_init
fio_uuid_register(void)
{
    register_unique(&unique);
}

static void fio_exit
fio_uuid_unregister(void)
{
    unregister_unique(&unique);
}
