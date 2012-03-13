/*
 * time + serial number based unique.
 */
#include "../fio.h"

static struct timeval last_time;
static uint32_t last_number;

static void
fio_time_number_set(struct fio_unique *u)
{
    if (last_number == 0) {
        fio_gettime(&last_time, NULL);
    }

    memcpy(&u->time_n.tv_sec, &last_time.tv_sec, sizeof(last_time.tv_sec));
    u->time_n.number = last_number;
    last_number++;
}

static void
fio_time_number_copy(struct fio_unique *udst, struct fio_unique *usrc)
{
    memcpy(&udst->time_n, &usrc->time_n, sizeof(usrc->time_n));
}

static char *
fio_time_number_to_string(struct fio_unique *u, char *s)
{
    snprintf(s, STRING_LEN, "%lu+%lu",
            (unsigned long)(u->time_n.tv_sec), (unsigned long)u->time_n.number);
    s[STRING_LEN-1] = '\0';
    return s;
}

static int
fio_time_number_is_match(struct fio_unique *u1, struct fio_unique *u2)
{
    return (u1->time_n.tv_sec == u2->time_n.tv_sec)&&(u1->time_n.number == u2->time_n.number);
}

static struct unique_ops unique = {
    .name       = "time_n",
    .set        = fio_time_number_set,
    .copy       = fio_time_number_copy,
    .to_string  = fio_time_number_to_string,
    .is_match   = fio_time_number_is_match,
};

static void fio_init
fio_time_number_register(void)
{
    register_unique(&unique);
}

static void fio_exit
fio_time_number_unregister(void)
{
    unregister_unique(&unique);
}
