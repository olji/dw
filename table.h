#ifndef TABLE_H
#define TABLE_H

struct table{
    /* Defines total depth of tree structure */
    short table_depth;
    short current_depth;

    /* Children of the node, represents numbers 0-9 */
    struct table *children[10];

    /* Word of leaf, will be NULL while current_depth < table_depth */
    char *word;
};

#endif
