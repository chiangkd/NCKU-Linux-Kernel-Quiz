#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _xorlist_node {
    struct _xorlist_node *cmp;
} xor_node_t;

typedef struct _xor_list_struct {
    xor_node_t head, tail;
    uint32_t count;
} xor_list_t;

#define XOR_COMP(a, b) ((xor_node_t *) ((uintptr_t)(a) ^ (uintptr_t)(b)))

static inline xor_node_t *address_of(xor_node_t *n1, xor_node_t *n2)
{
    assert(n1 && n2);
    return XOR_COMP(n1, n2);
}

#define container_of(ptr, type, member)                            \
    __extension__({                                                \
        const __typeof__(((type *) 0)->member) *__pmember = (ptr); \
        (type *) ((char *) __pmember - offsetof(type, member));    \
    })

#define xorlist_for_each(node, rp, rn, list)                        \
    for (rp = &(list)->head, node = rp->cmp; node != &(list)->tail; \
         rn = address_of(rp, node->cmp), rp = node, node = rn)

#define xorlist_for_each_prev(node, rp, rn, list)                   \
    for (rp = &(list)->tail, node = rp->cmp; node != &(list)->head; \
         rn = address_of(rp, node->cmp), rp = node, node = rn)

#define xorlist_for_each_from(node, pos1, pos2, rp, rn, list) \
    for (rp = pos2, node = pos1; node != &(list)->tail;       \
         rn = address_of(rp, node->cmp), rp = node, node = rn)

#define xorlist_for_each_from_prev(node, pos1, pos2, rp, rn, list) \
    for (rp = pos1, node = pos2; node != &(list)->head;            \
         rn = address_of(rp, node->cmp), rp = node, node = rn)

/* Note that when the delete function success is must return 0. */
#define xorlist_delete_prototype(name, node) \
    int _xorlist_delete_##name(xor_node_t *node)

#define xorlist_delete_call(name) _xorlist_delete_##name

static inline xor_node_t *xornode_init(xor_node_t *n)
{
    assert(n);
    n->cmp = NULL;
    return n;
}

#define XORNODE_INIT(n) \
    do {                \
        (n).cmp = NULL; \
    } while (0)

#define XORLIST_INIT(h)           \
    do {                          \
        (h).head.cmp = &(h).tail; \
        (h).tail.cmp = &(h).head; \
        (h).count = 0;            \
    } while (0)

int xorlist_add(xor_list_t *list, xor_node_t *n)
{
    xor_node_t *real_next;

    if (!n)
        return ENOMEM;

    xor_node_t *real_prev = &list->head;
    xor_node_t *node = real_prev->cmp;
    // if (node == &list->tail)
    //     real_next = &list->tail;
    // else
    //     real_next = node;
    real_next = node;
    real_prev->cmp = n;
    n->cmp = XOR_COMP(real_prev, real_next);
    real_next->cmp = XOR_COMP(n, XOR_COMP(real_prev, real_next->cmp));
    list->count++;

    return 0;
}

int xorlist_del(xor_list_t *list,
                xor_node_t *n,
                xor_node_t *target,
                int (*delete_func)(xor_node_t *))
{
    assert(list && n && target && delete_func);
    assert(&list->head != target && &list->tail != target);
    xor_node_t *nn = address_of(target, n->cmp);
    xor_node_t *an = address_of(n, target->cmp);
    xor_node_t *ana = address_of(target, an->cmp);
    n->cmp = XOR_COMP(nn, an);
    an->cmp = XOR_COMP(n, ana);
    delete_func(target);
    list->count--;

    return 0;
}

int xorlist_destroy(xor_list_t *list, int (*delete_func)(xor_node_t *))
{
    assert(delete_func);

    xor_node_t *real_prev = &list->head;
    xor_node_t *node = real_prev->cmp;
    xor_node_t *real_next = address_of(real_prev, node->cmp);
    xor_node_t *tmp = real_prev;
    real_prev = node;
    node = real_next;

    while (node != &list->tail) {
        real_next = address_of(real_prev, node->cmp);
        tmp = real_prev;
        real_prev = node;
        node = real_next;

        if (delete_func(tmp) != 0)
            perror("delete function failed");
    }

    return 0;
}

struct test_node {
    int value;
    xor_node_t xornode;
};

xorlist_delete_prototype(test_delete, _node)
{
    struct test_node *node = container_of(_node, struct test_node, xornode);
    free(node);
    return 0;
}

int main(void)
{
    xor_list_t list;
    xor_node_t *p1, *p2;

    XORLIST_INIT(list);
    for (int i = 0; i < 1000; i++) {
        struct test_node *new = malloc(sizeof(struct test_node));
        xornode_init(&new->xornode);
        new->value = i;
        xorlist_add(&list, &new->xornode);
        if (i == 5)
            p1 = &new->xornode;
        if (i == 6)
            p2 = &new->xornode;
    }

    xor_node_t *real_prev, *real_next, *node;
    int i = 0;
    printf("xorlist_for_each test\n");
    xorlist_for_each(node, real_prev, real_next, &list)
    {
        printf("node [%d] %d\n", i++,
               container_of(node, struct test_node, xornode)->value);
    }

    i = 0;
    printf("xorlist_for_from test\n");
    xorlist_for_each_from(node, p1, p2, real_prev, real_next, &list)
    {
        printf("node %d\n",
               container_of(node, struct test_node, xornode)->value);
    }

    i = 0;
    printf("xorlist_for_each_from_prev test\n");
    xorlist_for_each_from_prev(node, p1, p2, real_prev, real_next, &list)
    {
        printf("node [%d] %d\n", i++,
               container_of(node, struct test_node, xornode)->value);
    }

    i = 0;
    printf("xorlist_for_each_prev test\n");
    xorlist_for_each_prev(node, real_prev, real_next, &list)
    {
        printf("node [%d] %d\n", i++,
               container_of(node, struct test_node, xornode)->value);
    }

    printf("xorlist_del test\n");
    xorlist_del(&list, p2, p1, xorlist_delete_call(test_delete));
    i = 0;
    xorlist_for_each(node, real_prev, real_next, &list)
    {
        printf("node [%d] %d\n", i++,
               container_of(node, struct test_node, xornode)->value);
    }

    xorlist_destroy(&list, xorlist_delete_call(test_delete));

    return 0;
}