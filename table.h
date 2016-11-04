#ifndef TABLE_H
#define TABLE_H
#include <stdbool.h>
#include <math.h>
#include <string.h>

/*
 * Constant for now, when configuration is up and 
 * running remove KEY_BASE and use length of CHAR_SET for TABLE_SIZE
 */
#define KEY_LENGTH 3
#define CHAR_SET "0123456789"
#define CHAR_SET_LENGTH strlen(CHAR_SET)
#define TABLE_SIZE (int)(pow(CHAR_SET_LENGTH,KEY_LENGTH))

/* 
 * Key is identifier for the value, 
 * used when generating passphrase. Such as 
 * 111111:Phrase in diceware lists, where 111111 is the key.
 * struct node_value to be used when key generation is solved.
 */
struct node_value{
    char id[KEY_LENGTH+1];
    char *value;
};
/* Hash map used for dw list representations */
struct dw_node{
    int key;
    struct node_value value;
    struct dw_node *next;
};

struct dw_hashmap{
    struct dw_node **map;
};

bool map_filled();
int str_hash(char*,int);
char *gen_word_key();
/* Verify uniqueness of string in linked list */
bool dw_node_unique(struct dw_node*, char*);
/* Insert dw_node into existing linked list */
void dw_node_insert(struct dw_node**, struct dw_node*);
/* Delete dw_node list */
void dw_node_delete(struct dw_node*);

bool dw_map_insert(struct dw_hashmap*, char*);
char *dw_node_lookup(struct dw_node*, char*);
char *dw_map_lookup(struct dw_hashmap*, char*);
void dw_delete_node(struct dw_node*);
void dw_delete_map(struct dw_hashmap*);
#endif
