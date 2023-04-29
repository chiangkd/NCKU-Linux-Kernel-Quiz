#include <stdio.h>
#include <string.h>

#define LOG2_MAX_NODES 6

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define rb_node(x_type)       \
    struct {                  \
        x_type *left, *right; \
        bool red;             \
    }

/* Each node in the RB tree consumes at least 1 byte of space (for the
 * linkage if nothing else, so there are a maximum of 1 << (sizeof(void *)
 * << 3 rb) tree nodes in any process, and thus, at most that many in any
 * tree.
 *
 * Maximum number of bytes in a process: 1 << (sizeof(void *) << 3).
 * Log2 of maximum number of bytes in a process: sizeof(void *) << 3.
 * Maximum number of tree nodes in a process: 1 << (sizeof(void *) << 3) /
 * sizeof(tree_node).
 * Maximum number of tree nodes in a process is at most:
 *   1 << (sizeof(void *) << 3) / sizeof(rb_node(x_type)).
 * Log2 of maximum number of tree nodes in a process is at most:
 *   ?
 */
#define RB_LOG2_MAX_MEM_BYTES (sizeof(void *) << 3)

/* clang-format off */
#define RB_LOG2_MAX_NODES            \
    (RB_LOG2_MAX_MEM_BYTES -         \
     (sizeof(void *) >= 8   ? 4      \
      : sizeof(void *) >= 4 ? 3   \
                            : 2) - 1)
/* clang-format on */

/* The choice of algorithm bounds the depth of a tree to twice the binary log of
 * the number of elements in the tree; the following bound follows.
 */
#define RB_MAX_DEPTH (RB_LOG2_MAX_NODES << 1)

/* Root structure */
#define rb_tree(x_type) \
    struct {            \
        x_type *root;   \
    }

/* Left accessors */
#define rbtn_left_get(x_type, x_field, x_node) ((x_node)->x_field.left)
#define rbtn_left_set(x_type, x_field, x_node, x_left) \
    do {                                               \
        (x_node)->x_field.left = x_left;               \
    } while (0)

/* Right accessors */
#define rbtn_right_get(x_type, x_field, x_node) ((x_node)->x_field.right)
#define rbtn_right_set(x_type, x_field, x_node, x_right) \
    do {                                                 \
        (x_node)->x_field.right = x_right;               \
    } while (0)
/* Color accessors */
#define rbtn_red_get(x_type, x_field, x_node) ((x_node)->x_field.red)
#define rbtn_color_set(x_type, x_field, x_node, x_red) \
    do {                                               \
        (x_node)->x_field.red = (x_red);               \
    } while (0)
#define rbtn_red_set(x_type, x_field, x_node) \
    do {                                      \
        (x_node)->x_field.red = true;         \
    } while (0)
#define rbtn_black_set(x_type, x_field, x_node) \
    do {                                        \
        (x_node)->x_field.red = false;          \
    } while (0)

/* Node initializer */
#define rbt_node_new(x_type, x_field, x_rbt, x_node)     \
    do {                                                 \
        rbtn_left_set(x_type, x_field, (x_node), NULL);  \
        rbtn_right_set(x_type, x_field, (x_node), NULL); \
        rbtn_red_set(x_type, x_field, (x_node));         \
    } while (0)

/* Tree initializer */
#define rb_new(x_type, x_field, x_rbt) \
    do {                               \
        (x_rbt)->root = NULL;          \
    } while (0)

/* Internal utility macros. */
#define rbtn_rotate_left(x_type, x_field, x_node, r_node)         \
    do {                                                          \
        (r_node) = rbtn_right_get(x_type, x_field, (x_node));     \
        rbtn_right_set(x_type, x_field, (x_node),                 \
                       rbtn_left_get(x_type, x_field, (r_node))); \
        rbtn_left_set(x_type, x_field, (r_node), (x_node));       \
    } while (0)
#define rbtn_rotate_right(x_type, x_field, x_node, r_node)        \
    do {                                                          \
        (r_node) = rbtn_left_get(x_type, x_field, (x_node));      \
        rbtn_left_set(x_type, x_field, (x_node),                  \
                      rbtn_right_get(x_type, x_field, (r_node))); \
        rbtn_right_set(x_type, x_field, (r_node), (x_node));      \
    } while (0)

/* The rb_gen() macro generates a type-specific red-black tree implementation,
 * based on the above C macros.
 * Arguments:
 *
 *   x_attr:
 *     Function attribute for generated functions (ex: static).
 *   x_prefix:
 *     Prefix for generated functions (ex: ex_).
 *   x_rb_type:
 *     Type for red-black tree data structure (ex: ex_t).
 *   x_type:
 *     Type for red-black tree node data structure (ex: ex_node_t).
 *   x_field:
 *     Name of red-black tree node linkage (ex: ex_link).
 *   x_less:
 *     Node comparison function name, with the following prototype:
 *
 *     bool x_less(x_type *x_node, x_type *x_other);
 *                        ^^^^^^
 *                        or x_key
 *     Interpretation of comparison function return values:
 *        1 : x_node <  x_other
 *        0 : x_node >= x_other
 *     In all cases, the x_node or x_key macro argument is the first argument to
 *     the comparison function, which makes it possible to write comparison
 *     functions that treat the first argument specially.  x_less must be a
 * total order on values inserted into the tree -- duplicates are not allowed.
 *
 * Assuming the following setup:
 *
 *   typedef struct ex_node_s ex_node_t;
 *   struct ex_node_s {
 *       rb_node(ex_node_t) ex_link;
 *   };
 *   typedef rb_tree(ex_node_t) ex_t;
 *   rb_gen(static, ex_, ex_t, ex_node_t, ex_link, ex_less)
 *
 * The following API is generated:
 *
 *   static void
 *   ex_new(ex_t *tree);
 *       Description: Initialize a red-black tree structure.
 *       Args:
 *         tree: Pointer to an uninitialized red-black tree object.
 *
 *   static void
 *   ex_insert(ex_t *tree, ex_node_t *node);
 *       Description: Insert node into tree.
 *       Assumes that equal nodes are not yet in the tree. (Is it still true?)
 *       Args:
 *         tree: Pointer to an initialized red-black tree object.
 *         node: Node to be inserted into tree.
 */
#define rb_gen(x_attr, x_prefix, x_rbt_type, x_type, x_field, x_less) \
    typedef struct {                                                  \
        x_type *node;                                                 \
        bool less;                                                    \
    } x_prefix##path_entry_t;                                         \
    x_attr void x_prefix##new (x_rbt_type * rbtree)                   \
    {                                                                 \
        rb_new(x_type, x_field, rbtree);                              \
    }                                                                 \
    rb_gen_insert(x_attr, x_prefix, x_rbt_type, x_type, x_field, x_less)

#define rb_gen_insert(x_attr, x_prefix, x_rbt_type, x_type, x_field, x_less)  \
    x_attr void x_prefix##insert(x_rbt_type *rbtree, x_type *node)            \
    {                                                                         \
        x_prefix##path_entry_t path[RB_MAX_DEPTH];                            \
        x_prefix##path_entry_t *pathp;                                        \
        rbt_node_new(x_type, x_field, rbtree, node);                          \
        /* Wind. */                                                           \
        path->node = rbtree->root;                                            \
        for (pathp = path; pathp->node; pathp++) {                            \
            bool less = pathp->less = x_less(node, pathp->node);              \
            if (less) {                                                       \
                pathp[1].node = rbtn_left_get(x_type, x_field, pathp->node);  \
            } else {                                                          \
                pathp[1].node = rbtn_right_get(x_type, x_field, pathp->node); \
            }                                                                 \
        }                                                                     \
        pathp->node = node;                                                   \
        assert(!rbtn_left_get(x_type, x_field, node));                        \
        assert(!rbtn_right_get(x_type, x_field, node));                       \
        /* Unwind */                                                         \
        while (pathp-- != path) {                                             \
            x_type *cnode = pathp->node;                                      \
            if (pathp->less) {                                                \
                x_type *left = pathp[1].node;                                 \
                rbtn_left_set(x_type, x_field, cnode, left);                  \
                if (!rbtn_red_get(x_type, x_field, left))                     \
                    return;                                                   \
                x_type *leftleft = rbtn_left_get(x_type, x_field, left);      \
                if (leftleft && rbtn_red_get(x_type, x_field, leftleft)) {    \
                    /* Fix up 4-node. */                                      \
                    x_type *tnode;                                            \
                    rbtn_black_set(x_type, x_field, leftleft);                \
                    rbtn_rotate_right(x_type, x_field, cnode, tnode);         \
                    cnode = tnode;                                            \
                }                                                             \
            } else {                                                          \
                x_type *right = pathp[1].node;                                \
                rbtn_right_set(x_type, x_field, cnode, right);                \
                if (!rbtn_red_get(x_type, x_field, right))                    \
                    return;                                                   \
                x_type *left = rbtn_left_get(x_type, x_field, cnode);         \
                if (left && rbtn_red_get(x_type, x_field, left)) {            \
                    /* Split 4-node. */                                       \
                    rbtn_black_set(x_type, x_field, left);                    \
                    rbtn_black_set(x_type, x_field, right);                   \
                    rbtn_red_set(x_type, x_field, cnode);                     \
                } else {                                                      \
                    /* Lean left. */                                          \
                    x_type *tnode;                                            \
                    bool tred = rbtn_red_get(x_type, x_field, cnode);         \
                    rbtn_rotate_left(x_type, x_field, cnode, tnode);          \
                    rbtn_color_set(x_type, x_field, tnode, tred);             \
                    rbtn_red_set(x_type, x_field, cnode);                     \
                    cnode = tnode;                                            \
                }                                                             \
            }                                                                 \
            pathp->node = cnode;                                              \
        }                                                                     \
        /* Set root, and make it black. */                                    \
        rbtree->root = path->node;                                            \
        rbtn_black_set(x_type, x_field, rbtree->root);                        \
    }

/* Tree instantiation */

typedef struct node_s node_t;
struct node_s {
    rb_node(node_t) link;
    int value;
};

static bool node_less(const node_t *x_node, const node_t *x_other)
{
    return x_node->value < x_other->value;
}

typedef rb_tree(node_t) tree_t;
rb_gen(static, tree_, tree_t, node_t, link, node_less);

static tree_t tree;
static node_t nodes[1 << LOG2_MAX_NODES];

static int sum_subtree(node_t *node)
{
    int result = 0;
    while (node) {
        node_t *pre;
        if (!rbtn_left_get(node_t, link, node))
            goto do_print;
        for (pre = rbtn_left_get(node_t, link, node);
             rbtn_right_get(node_t, link, pre) &&
             rbtn_right_get(node_t, link, pre) != node;
             pre = rbtn_right_get(node_t, link, pre)) {
        }
        if (!rbtn_right_get(node_t, link, pre)) {
            rbtn_right_get(node_t, link, pre) = node;
            node = rbtn_left_get(node_t, link, node);
        } else {
            rbtn_right_get(node_t, link, pre) = NULL;

        do_print:
            result += node->value;
            printf("value: %d\n", node->value);
            node = rbtn_right_get(node_t, link, node);
        }
    }
    return result;
}

static node_t *lookup(node_t *node, int value)
{
    node_t tmp = {.value = value};
    while (node) {
        if (node_less(&tmp, node)) {
            node = rbtn_left_get(node_t, link, node);
        } else if (node_less(node, &tmp)) {
            node = rbtn_right_get(node_t, link, node);
        } else {
            break;
        }
    }
    return node;
}

int main(int argc, char **argv)
{
    tree_new(&tree);
    node_t *nodei = nodes;
    for (++argv; *argv; ++argv, ++nodei) {
        int v, n;
        if ((char *) nodei == (char *) nodes + sizeof(nodes)) {
            fprintf(stderr, "fatal: out of node memory\n");
            return 2;
        }
        if (sscanf(*argv, "%d%n", &v, &n) <= 0 || strlen(*argv) != n + 0U) {
            fprintf(stderr, "fatal: bad number in arg: %s\n", *argv);
            return 1;
        }
        nodei->value = v;
        tree_insert(&tree, nodei);
    }

    node_t *const node = lookup(tree.root, 11);
    printf("lookup(11): %s\n",
           (node && node->value == 11) ? "OK" : "Not found");

    const int sum = sum_subtree(tree.root);
    printf("sum: %d\n", sum);

    return 0;
}