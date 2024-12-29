#ifndef SPLAY_TREE_H
#define SPLAY_TREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#endif // SPLAY_TREE_H

#ifndef SPLAY_TREE_NAME
#error "Must define SPLAY_TREE_NAME"
#endif

#ifndef SPLAY_TREE_KEY_TYPE
#error "Must define SPLAY_TREE_TYPE"
#endif

#ifndef SPLAY_TREE_VALUE_TYPE
#error "Must define SPLAY_TREE_VALUE_TYPE"
#endif

#define SPLAY_CONCAT_(a, b) a ## b
#define SPLAY_CONCAT(a, b) SPLAY_CONCAT_(a, b)
#define SPLAY_TREE_TYPED(name) SPLAY_CONCAT(SPLAY_TREE_NAME, _##name)
#define SPLAY_TREE_FUNC(func) SPLAY_CONCAT(SPLAY_TREE_NAME, _##func)


#ifndef SPLAY_TREE_KEY_LESS_THAN
static inline bool SPLAY_TREE_TYPED(key_less_than)(SPLAY_TREE_KEY_TYPE key, SPLAY_TREE_KEY_TYPE node_key) {
    return key < node_key;
}
#define SPLAY_TREE_KEY_LESS_THAN SPLAY_TREE_TYPED(key_less_than)
#endif

#ifndef SPLAY_TREE_KEY_EQUALS
static inline bool SPLAY_TREE_TYPED(key_equals)(SPLAY_TREE_KEY_TYPE key, SPLAY_TREE_KEY_TYPE node_key) {
    return key == node_key;
}
#define SPLAY_TREE_KEY_EQUALS SPLAY_TREE_TYPED(key_equals)
#endif

typedef struct SPLAY_TREE_TYPED(node) {
    SPLAY_TREE_KEY_TYPE key;
    struct SPLAY_TREE_TYPED(node) *left;
    struct SPLAY_TREE_TYPED(node) *right;
    SPLAY_TREE_VALUE_TYPE value;
} SPLAY_TREE_TYPED(node_t);

#define SPLAY_TREE_NODE SPLAY_TREE_TYPED(node_t)

#define SPLAY_TREE_NODE_MEMORY_POOL_NAME SPLAY_TREE_TYPED(node_memory_pool)

#define MEMORY_POOL_NAME SPLAY_TREE_NODE_MEMORY_POOL_NAME
#define MEMORY_POOL_TYPE SPLAY_TREE_NODE
#include "memory_pool/memory_pool.h"
#undef MEMORY_POOL_NAME
#undef MEMORY_POOL_TYPE

#define SPLAY_NODE_MEMORY_POOL_FUNC(name) SPLAY_CONCAT(SPLAY_TREE_NODE_MEMORY_POOL_NAME, _##name)




typedef struct SPLAY_TREE_NAME {
    SPLAY_TREE_NODE *root;
    /* stack pool contains temporary nodes for the linked-list-based stack
       used in splaying. Here we expect the free list to be used much
       much more frequently since it is used for every search operation. */
    SPLAY_TREE_TYPED(node_memory_pool) *stack_pool;
    /* node pool contains all the nodes in the tree
       nodes are only allocated on insert and returned on delete
       so there should be less jumping around */
    SPLAY_TREE_TYPED(node_memory_pool) *node_pool;
} SPLAY_TREE_NAME;


SPLAY_TREE_NAME *SPLAY_TREE_FUNC(new)(void) {
    SPLAY_TREE_NAME *tree = calloc(1, sizeof(SPLAY_TREE_NAME));
    if (tree == NULL) return NULL;
    tree->node_pool = SPLAY_NODE_MEMORY_POOL_FUNC(new)();
    if (tree->node_pool == NULL) {
        free(tree);
        return NULL;
    }
    tree->stack_pool = SPLAY_NODE_MEMORY_POOL_FUNC(new)();
    if (tree->stack_pool == NULL) {
        SPLAY_NODE_MEMORY_POOL_FUNC(destroy)(tree->node_pool);
        free(tree);
        return NULL;
    }

    SPLAY_TREE_NODE *root = SPLAY_NODE_MEMORY_POOL_FUNC(get)(tree->node_pool);
    if (root == NULL) {
        SPLAY_NODE_MEMORY_POOL_FUNC(destroy)(tree->node_pool);
        free(tree);
        return NULL;
    }
    tree->root = root;
    tree->root->left = NULL;
    tree->root->right = NULL;
    tree->root->value = NULL;
    return tree;
}


void SPLAY_TREE_FUNC(rotate_left)(SPLAY_TREE_NODE *node) {
    /*
    Left rotation (rotate b to the left)

          b                   c
        /   \               /   \
    [a,b)    c      =>     b   [c,d)
           /   \         /   \
        [b,c) [c,d)   [a,b)  [b,c)

    Different from BST rotations though since we need
    to set the value pointer.
    */

    SPLAY_TREE_NODE *tmp_node = node->left;
    SPLAY_TREE_KEY_TYPE tmp_key = node->key;
    SPLAY_TREE_VALUE_TYPE tmp_value = node->value;
    node->left = node->right;
    node->key = node->right->key;
    node->value = node->right->value;

    node->right = node->left->right;
    node->left->right = node->left->left;

    node->left->left = tmp_node;
    node->left->key = tmp_key;
    node->left->value = tmp_value;
}


void SPLAY_TREE_FUNC(rotate_right)(SPLAY_TREE_NODE *node) {
    /*
    Right rotation (rotate c to the right)

            c                 b
          /   \             /   \ 
         b   [c,d)  =>  [a,b)    c
       /   \                   /   \
    [a,b)  [b,c)            [b,c)  [c,d)

    Different from BST rotations though since we need
    to set the value pointer.
    */

    SPLAY_TREE_NODE *tmp_node = node->right;
    SPLAY_TREE_KEY_TYPE tmp_key = node->key;
    SPLAY_TREE_VALUE_TYPE tmp_value = node->value;

    node->right = node->left;
    node->key = node->left->key;
    node->value = node->left->value;

    node->left = node->right->left;
    node->right->left = node->right->right;

    node->right->right = tmp_node;
    node->right->key = tmp_key;
    node->right->value = tmp_value;
}




void *SPLAY_TREE_FUNC(get)(SPLAY_TREE_NAME *tree, SPLAY_TREE_KEY_TYPE key) {
    if (tree == NULL || tree->root == NULL) return NULL;
    bool finished = false;
    if (tree->root->value == NULL) {
        // tree is empty
        return NULL;
    }
    SPLAY_TREE_NODE *root = tree->root;
    SPLAY_TREE_NODE *current_node = tree->root;
    SPLAY_TREE_NODE *stack = NULL;
    SPLAY_TREE_NODE *tmp_stack = NULL;
    while (!finished) {
        tmp_stack = SPLAY_NODE_MEMORY_POOL_FUNC(get)(tree->stack_pool);
        if (tmp_stack == NULL) return NULL;
        tmp_stack->left = current_node;
        tmp_stack->right = stack;
        stack = tmp_stack;
        if (SPLAY_TREE_KEY_LESS_THAN(key, current_node->key) && current_node->left != NULL) {
            current_node = current_node->left;
        } else if (!SPLAY_TREE_KEY_EQUALS(key, current_node->key) && current_node->right != NULL) {
            // key greater
            current_node = current_node->right;
        } else {
            // key equal
            finished = true;
        }
    }

    if (current_node->key != key) {
        // key not found
        return NULL;
    } else {
        SPLAY_TREE_NODE *parent, *grandparent;
        tmp_stack = stack;
        stack = stack->right;
        SPLAY_NODE_MEMORY_POOL_FUNC(release)(tree->stack_pool, tmp_stack);
        while (current_node != root) {
            parent = stack->left;
            tmp_stack = stack;
            stack = stack->right;
            SPLAY_NODE_MEMORY_POOL_FUNC(release)(tree->stack_pool, tmp_stack);
            if (parent == root) {
                // zig
                if (parent->left == current_node) {
                    SPLAY_TREE_FUNC(rotate_right)(parent);
                } else {
                    SPLAY_TREE_FUNC(rotate_left)(parent);
                }
                current_node = parent;
            } else {
                grandparent = stack->left;
                tmp_stack = stack;
                stack = stack->right;
                SPLAY_NODE_MEMORY_POOL_FUNC(release)(tree->stack_pool, tmp_stack);
                if (grandparent->left == parent) {
                    // zig zig
                    if (parent->left == current_node) {
                        SPLAY_TREE_FUNC(rotate_right)(grandparent);
                    } else {
                        SPLAY_TREE_FUNC(rotate_left)(parent);
                    }
                    SPLAY_TREE_FUNC(rotate_right)(grandparent);
                } else {
                    // zig zag
                    if (parent->right == current_node) {
                        SPLAY_TREE_FUNC(rotate_left)(grandparent);
                    } else {
                        SPLAY_TREE_FUNC(rotate_right)(parent);
                    }
                    SPLAY_TREE_FUNC(rotate_left)(grandparent);

                }
                current_node = grandparent;
            }
        }
        return current_node->value;
    }
    return NULL;
}

bool SPLAY_TREE_FUNC(insert_with_options)(SPLAY_TREE_NAME *tree, SPLAY_TREE_KEY_TYPE key, SPLAY_TREE_VALUE_TYPE value, bool replace) {
    if (tree == NULL || tree->root == NULL) return false;
    if (tree->root->value == NULL) {
        // tree is empty
        tree->root->key = key;
        tree->root->value = value;
        tree->root->left = NULL;
        tree->root->right = NULL;
        return true;
    }

    SPLAY_TREE_NODE *next = tree->root;
    SPLAY_TREE_NODE *parent = NULL;

    while (next != NULL) {
        parent = next;
        if (SPLAY_TREE_KEY_LESS_THAN(key, next->key)) {
            next = next->left;
        } else if (!SPLAY_TREE_KEY_EQUALS(key, next->key)) {
            next = next->right;
        } else {
            // Key equal
            if (replace) {
                next->value = value;
            }
            return false;
        }
    }

    SPLAY_TREE_NODE *new_leaf = SPLAY_NODE_MEMORY_POOL_FUNC(get)(tree->node_pool);
    new_leaf->value = value;
    new_leaf->key = key;
    new_leaf->left = NULL;
    new_leaf->right = NULL;
    if (key < parent->key) {
        parent->left = new_leaf;
    } else {
        parent->right = new_leaf;
    }
    return true;
}

bool SPLAY_TREE_FUNC(insert)(SPLAY_TREE_NAME *tree, SPLAY_TREE_KEY_TYPE key, SPLAY_TREE_VALUE_TYPE value) {
    bool replace = true;
    return SPLAY_TREE_FUNC(insert_with_options)(tree, key, value, replace);
}

bool SPLAY_TREE_FUNC(insert_no_replace)(SPLAY_TREE_NAME *tree, SPLAY_TREE_KEY_TYPE key, SPLAY_TREE_VALUE_TYPE value) {
    bool replace = false;
    return SPLAY_TREE_FUNC(insert_with_options)(tree, key, value, replace);
}

void *SPLAY_TREE_FUNC(delete)(SPLAY_TREE_NAME *tree, SPLAY_TREE_KEY_TYPE key) {
    if (tree == NULL || tree->root == NULL) return 0;
    if (tree->root->value == NULL) {
        // tree is empty
        return NULL;
    }
    SPLAY_TREE_NODE *parent = NULL;
    SPLAY_TREE_NODE *tmp = NULL;
    SPLAY_TREE_NODE *next = tree->root;

    while (next != NULL) {
        parent = tmp;
        tmp = next;
        if (SPLAY_TREE_KEY_LESS_THAN(key, tmp->key)) {
            next = tmp->left;
        } else if (!SPLAY_TREE_KEY_EQUALS(key, tmp->key)) {
            next = tmp->right;
        } else {
            // Found key
            break;
        }
    }

    if (next == NULL) {
        // Key not found
        return NULL;
    }
    void *deleted = tmp->value;
    SPLAY_TREE_NODE *deleted_node = NULL;
    if (tmp->left == NULL && tmp->right == NULL) {
        // degree 0 node: delete
        if (parent != NULL) {
            if (tmp == parent->left) {
                parent->left = NULL;
            } else{
                parent->right = NULL;
            }
            SPLAY_NODE_MEMORY_POOL_FUNC(release)(tree->node_pool, tmp);
        } else {
            // make tree empty
            tmp->value = NULL;
        }
    } else if (tmp->left == NULL) {
        tmp->left = tmp->right->left;
        tmp->key = tmp->right->key;
        tmp->value = tmp->right->value;
        deleted_node = tmp->right;
        tmp->right = tmp->right->right;
        SPLAY_NODE_MEMORY_POOL_FUNC(release)(tree->node_pool, deleted_node);
    } else if (tmp->right == NULL) {
        tmp->right = tmp->left->right;
        tmp->key = tmp->left->key;
        tmp->value = tmp->left->value;
        deleted_node = tmp->left;
        tmp->left = tmp->left->left;
        SPLAY_NODE_MEMORY_POOL_FUNC(release)(tree->node_pool, deleted_node);
    } else {
        // interior node needs to be deleted
        parent = tmp;
        deleted_node = tmp->right;
        while (deleted_node->left != NULL) {
            parent = deleted_node;
            deleted_node = deleted_node->left;
        }
        tmp->key = deleted_node->key;
        tmp->value = deleted_node->value;
        if (deleted_node == tmp->right) {
            tmp->right = deleted_node->right;
        } else {
            parent->left = deleted_node->right;
        }
        SPLAY_NODE_MEMORY_POOL_FUNC(release)(tree->node_pool, deleted_node);
    }
    return deleted;
}

void SPLAY_TREE_FUNC(destroy)(SPLAY_TREE_NAME *tree) {
    if (tree == NULL) return;
    if (tree->node_pool != NULL) {
        SPLAY_NODE_MEMORY_POOL_FUNC(destroy)(tree->node_pool);
    }
    free(tree);
}




