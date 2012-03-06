/*
 * time based unique.
 */
#include "../fio.h"

static void
fio_time_set(struct fio_unique *u)
{
    fio_gettime(&u->time, NULL);
}

static void
fio_time_copy(struct fio_unique *udst, struct fio_unique *usrc)
{
    memcpy(&udst->time, &usrc->time, sizeof(usrc->time));
}

static char *
fio_time_to_string(struct fio_unique *u, char *s)
{
    snprintf(s, STRING_LEN, "%u/%u", u->time.tv_sec, u->time.tv_usec);
    s[STRING_LEN-1] = '\0';
    return s;
}

static int
fio_time_is_match(struct fio_unique *u1, struct fio_unique *u2)
{
    return (u1->time.tv_sec == u2->time.tv_sec)&&(u1->time.tv_usec == u2->time.tv_usec);

}

static struct unique_ops unique = {
    .name       = "time",
    .set        = fio_time_set,
    .copy       = fio_time_copy,
    .to_string  = fio_time_to_string,
    .is_match   = fio_time_is_match,
};

static void fio_init
fio_time_register(void)
{
    register_unique(&unique);
}

static void fio_exit
fio_time_unregister(void)
{
    unregister_unique(&unique);
}
