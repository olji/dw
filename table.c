#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "table.h"
#include "config.h"

extern struct dw_config CONFIG;

static char *key;
int str_hash(char *word, int modval){
    int pos = 0;
    for (int i=0; word[i] != '\0'; ++i){
        word[i] = tolower(word[i]);
        pos += word[i];
    }
    return pos%modval;
}
void init_key(){
    key = calloc(CONFIG.key_length + 1, sizeof(char));
    for (int i = 0; i < CONFIG.key_length; ++i){
        key[i] = CONFIG.char_set[0];
    }
}
bool map_filled(){
    char *last_key = calloc(CONFIG.key_length + 1, sizeof(char));
    for (int i = 0; i < CONFIG.key_length; ++i){
        last_key[i] = CONFIG.char_set[CONFIG.char_set_size-1];
    }
    bool retval = (strcmp(key, last_key) == 0);
    free(last_key);
    return retval;
}
void increment_key(){
    int key_pos = 0;
    for (;;){
        int pos = strchr(CONFIG.char_set, key[key_pos])-CONFIG.char_set;
        if (pos < CONFIG.char_set_size - 1){
            key[key_pos] = CONFIG.char_set[pos+1];
            break;
        } else {
            key[key_pos] = CONFIG.char_set[0];
            ++key_pos;
            if (key_pos > CONFIG.key_length){
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
    int pos = str_hash(word, CONFIG.table_size);
    bool uniq = dw_node_unique(map->map[pos], word);
    if (!CONFIG.unique || uniq){
        char *given_key = gen_word_key();
        struct dw_node *new = malloc(sizeof(struct dw_node));
        new->key = pos;
        new->value.id = malloc(sizeof(char)*(CONFIG.key_length+1));
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
    if (strcmp(list->value.value, str) == 0){
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
char *dw_node_lookup(struct dw_node *node, char *id){
    if (node == NULL){
        return "";
    }
    if (strcmp(node->value.id, id) == 0){
        return node->value.value;
    }
    return dw_node_lookup(node->next, id);
}
char *dw_map_lookup(struct dw_hashmap *list, char *id){
    char *ret = dw_node_lookup(list->map[str_hash(id, CONFIG.table_size)], id);
    if (strcmp(ret, "") == 0){
        printf("ERR: Could not find '%s' in hash %d\n", id, str_hash(id, CONFIG.table_size));
    }
    return ret;
}
void dw_delete_node(struct dw_node* node){
    if (node->next != NULL){
        dw_delete_node(node->next);
    }
    free(node->value.id);
    free(node->value.value);
    free(node->next);
}
void dw_delete_map(struct dw_hashmap* list){
    for (int i = 0; i < CONFIG.table_size; ++i){
        if (list->map[i] != NULL){
            dw_delete_node(list->map[i]);
            free(list->map[i]);
        }
    }
    free(list->map);
    free(list);
}
