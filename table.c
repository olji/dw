#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "table.h"

bool dw_map_insert(struct dw_hashmap *map, char *word){
    /* 
     * 1. Hash data for position in table (Make data all lowercase first)
     * 2. Check all other data in position using strcmp() or similar
     * 3. If unique, insert to table and return 1, if not, return 0.
     */
    int pos;
    for (int i=0; word[i] != '\0'; ++i){
        word[i] = tolower(word[i]);
        pos += word[i];
    }
    pos = pos%TABLE_SIZE;
    bool uniq = dw_node_unique(map->map[pos], word);
    if (uniq){
        printf("Hey: %s\n", word);
        struct dw_node *new = malloc(sizeof(struct dw_node));
        new->hashed_key = pos;
        new->value = word; 
        new->next = NULL;
        dw_node_insert(&map->map[pos], new);
    }

    return uniq;
}
bool dw_node_unique(struct dw_node *list, char *str){
    if (list == NULL){
        return true;
    } 
    else if (strcmp(list->value, str) == 0){
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
void dw_node_delete(struct dw_node *list){
}
