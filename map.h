#ifndef MAP_H
#define MAP_H
#include <stdbool.h>
#include <math.h>
#include <string.h>

/* 
 * Key is identifier for the value, 
 * used when generating passphrase. Such as 
 * 111111:Phrase in diceware lists, where 111111 is the key.
 * struct node_value to be used when key generation is solved.
 */
struct node_value{
    char *id;
    char *value;
};
/* Hash map used for dw list representations */
struct dw_node{
    size_t key;
    struct node_value value;
    struct dw_node *next;
};

struct dw_hashmap{
    struct dw_node **map;
};

bool map_filled();
int str_hash(char*,int);
char *gen_word_key();

bool  map_insert(struct dw_hashmap*, char*);
char *map_lookup(struct dw_hashmap*, char*);
void  map_free(struct dw_hashmap*);
void  map_write(FILE*, struct dw_hashmap*);
void  map_print(struct dw_hashmap *table);
void  map_rearrange(struct dw_hashmap*);

void  node_insert(struct dw_node**, struct dw_node*);
void  node_write(FILE*, struct dw_node*);
void  node_print(struct dw_node *node);
int   node_count(struct dw_node *node);
void  node_free(struct dw_node*);
bool  node_unique(struct dw_node*, char*);
char *node_lookup(struct dw_node*, char*);
void  node_rearrange(struct dw_node**, struct dw_node*);

#endif
