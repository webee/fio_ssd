/*
 * generate contextless global unique code.
 * related operations.
 */
#include "fio.h"

static FLIST_HEAD(unique_list);

static struct unique_ops *
find_unique(const char *name)
{
    struct unique_ops *ops;
    struct flist_head *entry;

    flist_for_each(entry, &unique_list) {
        ops = flist_entry(entry, struct unique_ops, list);
        if (!strcmp(name, ops->name))
            return ops;
    }
    return NULL;
}

void
register_unique(struct unique_ops *ops)
{
    dprint(FD_VERIFY, "uniuqe %s registered\n", ops->name);
    INIT_FLIST_HEAD(&ops->list);
    flist_add_tail(&ops->list, &unique_list);
}

void
unregister_unique(struct unique_ops *ops)
{
    dprint(FD_VERIFY, "uniuqe %s unregistered\n", ops->name);
    flist_del(&ops->list);
    INIT_FLIST_HEAD(&ops->list);
}

struct unique_ops *
load_unique(struct thread_data *td, const char *name)
{
    struct unique_ops *ops, *ret;
    char unique[NAME_LEN];

    dprint(FD_VERIFY, "uniuqe %s loading.\n", td->unique_ops->name);

    strncpy(unique, name, sizeof(unique)-1);

    ops = find_unique(unique);
    if (!ops) {
        log_err("fio: unique %s not loadable\n", name);
        return NULL;
    }

    ret = malloc(sizeof(*ret));
    memcpy(ret, ops, sizeof(*ret));

    return ret;
}

void
free_unique(struct thread_data *td)
{
    dprint(FD_VERIFY, "close uniuqe %s\n", td->unique_ops->name);
    if (td->unique_ops)
        free(td->unique_ops);
    td->unique_ops = NULL;
}

void
close_unique(struct thread_data *td)
{
    dprint(FD_VERIFY, "uniuqe %s closing\n", td->unique_ops->name);
    
    free_unique(td);
}
