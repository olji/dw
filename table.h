#ifndef TABLE_H
#define TABLE_H
#include <stdbool.h>

/* Constant for now, make dynamic with size n^m, where n = key length and m = key character set size */
#define TABLE_SIZE 4000
/* 
 * Key is identifier for the value, used when generating passphrase. Such as 111111:Phrase in diceware lists, where 111111 is the key.
 * struct node_value to be used when key generation is solved.
 */
struct node_value{
    char *id;
    char *value;
};
/* Hash map used for dw list representations */
struct dw_node{
    int key;
    char *value;
    struct dw_node *next;
};

struct dw_hashmap{
    struct dw_node *map[TABLE_SIZE];
};

/* Verify uniqueness of string in linked list */
bool dw_node_unique(struct dw_node*, char*);
/* Insert dw_node into existing linked list */
void dw_node_insert(struct dw_node**, struct dw_node*);
/* Delete dw_node list */
void dw_node_delete(struct dw_node*);

bool dw_map_insert(struct dw_hashmap*, char*);
char *dw_map_lookup(struct dw_hashmap*, char*);
bool dw_delete_map(struct dw_hashmap*);
#endif
