#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "table.h"

static char key[KEY_LENGTH];
void init_key(){
    for (int i = 0; i < KEY_LENGTH; ++i){
        key[i] = CHAR_SET[0];
    }
}
bool map_filled(){
    char last_key[KEY_LENGTH];
    for (int i = 0; i < KEY_LENGTH; ++i){
        last_key[i] = CHAR_SET[CHAR_SET_LENGTH-1];
    }
    return (strcmp(key, last_key) == 0);
}
void increment_key(){
    int key_pos = 0;
    for (;;){
        int pos = strchr(CHAR_SET, key[key_pos])-CHAR_SET;
        if (pos < CHAR_SET_LENGTH - 1){
            key[key_pos] = CHAR_SET[pos+1];
            break;
        } else {
            key[key_pos] = CHAR_SET[0];
            ++key_pos;
            if (key_pos > KEY_LENGTH){
                break;
            }
        }
    }
}
char *gen_word_key(){
    static bool first_run = true;
    if (first_run){
        init_key();
        first_run = false;
    } else {
        increment_key();
    }
    return key;
}
bool dw_map_insert(struct dw_hashmap *map, char *word){
    int pos;
    for (int i=0; word[i] != '\0'; ++i){
        word[i] = tolower(word[i]);
        pos += word[i];
    }
    pos = pos%TABLE_SIZE;
    bool uniq = dw_node_unique(map->map[pos], word);
    if (uniq){
        char *given_key = gen_word_key();
        struct dw_node *new = malloc(sizeof(struct dw_node));
        new->key = pos;
        strcpy(new->value.id, given_key);
        new->value.value = malloc(sizeof(char)*strlen(word)+1);
        strcpy(new->value.value, word);
        new->next = NULL;
        dw_node_insert(&map->map[pos], new);
    }

    return uniq;
}
bool dw_node_unique(struct dw_node *list, char *str){
    if (list == NULL){
        return true;
    } 
    else if (strcmp(list->value.value, str) == 0){
        return false;
    }
    return dw_node_unique(list->next, str);
}
void dw_node_insert(struct dw_node **list, struct dw_node *node){
    if (*list == NULL){
        *list = node;
    } else {
        dw_node_insert(&(*list)->next, node);
    }
}
char *dw_map_lookup(struct dw_hashmap*, char*);
void dw_delete_node(struct dw_node* node){
    if (node->next == NULL){
        free(node->value.value);
        return;
    }
    dw_delete_node(node->next);
    free(node->next);
}
void dw_delete_map(struct dw_hashmap* list){
    for (int i = 0; i < TABLE_SIZE; ++i){
        if (list->map[i] != NULL){
            dw_delete_node(list->map[i]);
        }
        free(list->map[i]);
    }
    free(list->map);
    free(list);
}
