/*
 * time + serial number based unique.
 */
#include "../fio.h"

#define timeval_match(t1,t2) (((t1).tv_sec == (t2).tv_sec)&&((t1).tv_usec == (t2).tv_usec))

static struct time_number last_unique;

static void
fio_time_number_set(struct fio_unique *u)
{
    fio_gettime(&(u->time_n.time), NULL);
    if (timeval_match(u->time_n.time, last_unique.time)) {
        last_unique.number++;
    }else {
        memcpy(&last_unique.time, &(u->time_n.time), sizeof(last_unique.time));
        last_unique.number=0;
    }
    u->time_n.number = last_unique.number;
}

static void
fio_time_number_copy(struct fio_unique *udst, struct fio_unique *usrc)
{
    memcpy(&udst->time_n, &usrc->time_n, sizeof(usrc->time_n));
}

static char *
fio_time_number_to_string(struct fio_unique *u, char *s)
{
    snprintf(s, STRING_LEN, "%lu/%lu+%u",
            (unsigned long)(u->time_n.time.tv_sec), (unsigned long)(u->time_n.time.tv_usec),
            u->time_n.number);
    s[STRING_LEN-1] = '\0';
    return s;
}

static int
fio_time_number_is_match(struct fio_unique *u1, struct fio_unique *u2)
{
    return timeval_match(u1->time_n.time, u2->time_n.time)&&(u1->time_n.number == u2->time_n.number);

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
