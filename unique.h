#ifndef FIO_UNIQUE_H
#define FIO_UNIQUE_H

#include <sys/time.h>

#define FIO_DEF_UNIQUE "time"

#define NAME_LEN    16
#define STRING_LEN    32

#define fio_string_unique(s)\
    char s[STRING_LEN]

struct fio_unique {
    union {
        struct timeval time;
    };
};

struct unique_ops {
    struct flist_head list;
    char name[NAME_LEN];
    void (*set)(struct fio_unique *u);
    void (*copy)(struct fio_unique *udst, struct fio_unique *usrc);
    char *(*to_string)(struct fio_unique *u, char *s);
    int (*is_match)(struct fio_unique *u1, struct fio_unique *u2);
};

extern void register_unique(struct unique_ops *ops);
extern void unregister_unique(struct unique_ops *ops);
extern struct unique_ops *load_unique(struct thread_data *td, const char *name);
void free_unique(struct thread_data *td);
extern void close_unique(struct thread_data *td);

#endif /* _UNIQUE_H */
