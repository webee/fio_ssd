#ifndef FIO_IHASH_H
#define FIO_IHASH_H

typedef struct _hash_node {
    struct _hash_node *next;
    void *key;
    void *value;
}HashNode;

typedef int (*HashFunc)(const void *key);
typedef int (*HashMatch)(const void *lkey, const void *rkey);
typedef void (*HashVisit)(void *value, void *context);

typedef struct _hash_map {
    HashNode **buckets;
    size_t size;
    int quantity;
    HashFunc hashfunc;
    HashMatch hashmatch;
}HashMap;

void hash_init(HashMap *thiz, size_t size, HashFunc hashfunc, HashMatch hashmatch);
void hash_destroy(HashMap *thiz);
void hash_clear(HashMap *thiz);

HashNode *hash_add(HashMap *thiz, const void *key, const void *value);
HashNode *hash_find(HashMap *thiz, const void *key);
int hash_del(HashMap *thiz, const void *key);
void *hash_value(HashMap *thiz, const void *key);
void hash_travel(HashMap *thiz, HashVisit visit, void *context);

/* hash functions */
int hashfunc_str(const void *s);
int hashfunc_prime(const void *s);
int hashfunc_elf(const void *s);
int hashfunc_uint64(const void *u);

/* hash match functions */
int hashmatch_str(const void *s1, const void *s2);
int hashmatch_strcase(const void *s1, const void *s2);
int hashmatch_uint64(const void *u1, const void *u2);

#endif // FIO_IHASH_H
