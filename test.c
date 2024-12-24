#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "greatest/greatest.h"

#define SPLAY_TREE_NAME splay_tree_uint32
#define SPLAY_TREE_KEY_TYPE uint32_t
#define SPLAY_TREE_VALUE_TYPE char *
#include "splay_tree.h"
#undef SPLAY_TREE_NAME
#undef SPLAY_TREE_KEY_TYPE
#undef SPLAY_TREE_VALUE_TYPE

TEST test_splay_tree(void) {
    splay_tree_uint32 *tree = splay_tree_uint32_new();

    char *val1 = "a";
    char *val2 = "b";
    char *val3 = "c";

    splay_tree_uint32_insert(tree, 1, "a");
    splay_tree_uint32_insert(tree, 5, "c");
    splay_tree_uint32_insert(tree, 3, "b");
    splay_tree_uint32_insert(tree, 9, "e");
    splay_tree_uint32_insert(tree, 7, "d");
    splay_tree_uint32_insert(tree, 11, "f");

    char *a = splay_tree_uint32_search(tree, 1);
    ASSERT_STR_EQ(a, "a");

    char *b = splay_tree_uint32_search(tree, 3);
    ASSERT_STR_EQ(b, "b");

    char *c = splay_tree_uint32_search(tree, 5);
    ASSERT_STR_EQ(c, "c");

    char *d = splay_tree_uint32_search(tree, 7);
    ASSERT_STR_EQ(d, "d");

    char *e = splay_tree_uint32_search(tree, 9);
    ASSERT_STR_EQ(e, "e");

    a = splay_tree_uint32_delete(tree, 1);
    ASSERT_STR_EQ(a, "a");
  
    a = splay_tree_uint32_search(tree, 1);
    ASSERT(a == NULL);
  
    b = splay_tree_uint32_delete(tree, 3);
    ASSERT_STR_EQ(b, "b");

    e = splay_tree_uint32_delete(tree, 9);
    ASSERT_STR_EQ(e, "e");

    c = splay_tree_uint32_search(tree, 5);
    ASSERT_STR_EQ(c, "c");

    c = splay_tree_uint32_delete(tree, 5);
    ASSERT_STR_EQ(c, "c");

    d = splay_tree_uint32_delete(tree, 7);
    ASSERT_STR_EQ(d, "d");

    d = splay_tree_uint32_search(tree, 7);
    ASSERT(d == NULL);

    splay_tree_uint32_insert(tree, 7, "d");
    d = splay_tree_uint32_search(tree, 7);

    splay_tree_uint32_destroy(tree);
    PASS();
}



/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */

    RUN_TEST(test_splay_tree);

    GREATEST_MAIN_END();        /* display results */
}