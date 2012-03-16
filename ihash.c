#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>

#include "fio.h"
#include "smalloc.h"
#include "ihash.h"

void
hash_init(HashMap *thiz, size_t size, HashFunc hashfunc, HashMatch hashmatch)
{
    thiz->buckets = (HashNode **)smalloc(sizeof(HashNode *) * size);
    memset(thiz->buckets, 0, sizeof(HashNode *) * size);
    thiz->size = size;
    thiz->quantity = 0;

    thiz->hashfunc = hashfunc;
    thiz->hashmatch = hashmatch;
}

void
hash_clear(HashMap *thiz)
{
    int i,n;
    HashNode *hn;
    for (i=n=0; thiz->quantity > 0 && i < thiz->size; i++) {
        hn = thiz->buckets[i];
        while (thiz->quantity > 0 && hn != NULL) {
            sfree(hn);
            thiz->quantity--;
            hn = hn->next;
        }
        thiz->buckets[i] = NULL;
    }
    thiz->quantity = 0;
    return;
}

void
hash_destroy(HashMap *thiz)
{
    hash_clear(thiz);
    sfree(thiz->buckets);
}

HashNode *
hash_add(HashMap *thiz, const void *key, const void *value)
{
    int pos = thiz->hashfunc(key) % thiz->size;
    HashNode *hn = (HashNode *)smalloc(sizeof(HashNode));
    if (hn) {
        hn->key = (void *)key;
        hn->value = (void *)value;
        hn->next = thiz->buckets[pos];
        thiz->buckets[pos] = hn;
        thiz->quantity++;
    }

    return hn;
}

HashNode *
hash_find(HashMap *thiz, const void *key)
{
    int pos = thiz->hashfunc(key) % thiz->size;
    HashNode *hn = thiz->buckets[pos];

    while (hn) {
        if (thiz->hashmatch(hn->key, key)) {
            return hn;
        }
        hn = hn->next;
    }
    return NULL;
}

int
hash_del(HashMap *thiz, const void *key)
{
    int pos = thiz->hashfunc(key) % thiz->size;
    HashNode *phn = NULL;
    HashNode *hn = thiz->buckets[pos];

    while (hn) {
        if (thiz->hashmatch(hn->key, key))
            break;
        phn = hn;
        hn = hn->next;
    }
    if (hn) {
        if (phn) {
            phn->next = hn->next;
        }else {
            thiz->buckets[pos] = hn->next;
        }
        sfree(hn);
        thiz->quantity--;
        return 0;
    }
    return 1;
}

void *
hash_value(HashMap *thiz, const void *key)
{
    HashNode *hn = (HashNode *)hash_find(thiz, key);
    if (hn)
        return (void *)hn->value;
    return NULL;
}

void
hash_travel(HashMap *thiz, HashVisit visit, void *context)
{
    int i,n;
    HashNode *hn, *hn2;
    for (i=n=0; n < thiz->quantity && i < thiz->size; i++) {
        hn = thiz->buckets[i];
        while (n < thiz->quantity && hn != NULL) {
            hn2 = hn->next;
            visit(hn->value, context);
            hn = hn2;
            n++;
        }
    }
}

/*
 * 37-300, 132 are ok
 * 37-1025 852 are ok
 * all is 37, used >= 34
 */
int
hashfunc_str(const void *str)
{
    char *s = (char *)str;
    int even, odd;
    int i = 0;
    int mask = 0x1F;
    int ret;
    even = odd = 0;
    while (*s != '\0') {
        if (i&1) odd ^= *s;
        else even ^= *s;
        ++s;
        ++i;
    }
    ret = even & mask;
    ret <<= 5;
    ret += (odd & mask);
    return ret;
}

/*
 * 37-300, 100 are ok
 * 37-1025 822 are ok
 * all is 37, used >= 34
 */
int
hashfunc_prime(const void *str)
{
    char *s = (char *)str;
    /*2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31 */
    int primeflag[32] = {
        0, 0, 1, 1, 0, 1, 0, 1,
        0, 0, 0, 1, 0, 1, 0, 0,
        0, 1, 0, 1, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 1, 0, 1,
    };
    int mask = 0x1F;
    int i = 0;
    int pValue = 0;
    int nValue = 0;
    int ret;
    while (*s) {
        if (i < 32 && primeflag[i]) pValue ^= *s;
        else nValue &= *s;
        ++s;
        ++i;
    }
    ret = pValue & mask;
    ret <<= 5;
    ret += (nValue & mask);
    return ret;
}

/*
 * 37-300, 127 are ok
 * 37-1025 837 are ok
 * all is 37, used >= 34
 */
int
hashfunc_elf(const void *str)
{
    char *s = (char *)str;
    unsigned long h,g;
    h = 0;
    while (*s) {
        h = (h<<4) + *s++;
        g = h & 0xf0000000;
        if (g) h ^= (g>>24);
        h &= ~g;
    }
    return h;
}

int
hashfunc_uint64(const void *ui)
{
    uint64_t *u = (uint64_t *)ui;
    return (*u)&(0xFFFFFFFF>>1);
}

/* hash match functions */
int
hashmatch_str(const void *str1, const void *str2)
{
    char *s1 = (char *)str1;
    char *s2 = (char *)str2;
    return strcmp(s1, s2)==0;
}

int
hashmatch_strcase(const void *str1, const void *str2)
{
    char *s1 = (char *)str1;
    char *s2 = (char *)str2;
    return strcasecmp(s1, s2)==0;
}

int
hashmatch_uint64(const void *ui1, const void *ui2)
{
    uint64_t *u1 = (uint64_t *)ui1;
    uint64_t *u2 = (uint64_t *)ui2;
    return *u1 == *u2;
}
