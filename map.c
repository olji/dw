/*
 * Copyright (C) 2016 Rickard S. Jonsson
 *
 * This file is part of dw.
 *
 * dw is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dw is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dw.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "map.h"
#include "config.h"
#include "ioput.h"
#include "mem.h"

extern struct dw_config CONFIG;

void increment_key(char *key){
    int key_pos = 0;
    for (;;){
        /* Get key[key_pos]'s position in character set */
        int pos = strchr(CONFIG.char_set, key[key_pos])-CONFIG.char_set;
        /* If lower than character set size, increment the character */
        if (pos < CONFIG.char_set_size - 1){
            key[key_pos] = CONFIG.char_set[pos+1];
            break;
        } else {
            /* Otherwise, set character to the first character in
             * character set and start looking at the next character in key */
            key[key_pos] = CONFIG.char_set[0];
            ++key_pos;
            if (key_pos > CONFIG.key_length){
                /* If key_pos is larger than key length, no more keys can be gotten */
                break;
            }
        }
    }
}
int str_hash(char *word, int modval){
    /* Take the sum of each character in the
     * word and screw around with it a bit */
    size_t pos = 0;
    for (int i=0; word[i] != '\0'; ++i){
        word[i] = tolower(word[i]);
        pos += word[i];
        pos += pos << 2;
        pos -= pos >> 3;
        pos ^= pos << 4;
    }
    return pos % modval;
}
void init_key(char **key){
    (*key) = str_malloc(CONFIG.key_length);
    /* Fill with first character in character set */
    /* TODO: Change loop to memset */
    for (int i = 0; i < CONFIG.key_length; ++i){
        (*key)[i] = CONFIG.char_set[0];
    }
}
bool map_filled(char *key){
    char *last_key = str_malloc(strlen(key));
    /* TODO: Change loop to memset */
    for (int i = 0; i < CONFIG.key_length; ++i){
        last_key[i] = CONFIG.char_set[CONFIG.char_set_size-1];
    }

    bool retval = (strcmp(key, last_key) == 0);
    free(last_key);
    return retval;
}
size_t map_left(char *key){
    size_t size = 0;
    char *tmp = str_malloc(strlen(key));
    strcpy(tmp, key);
    /* Keep track of how many times tmp has to be incremented to be seen as full */
    while(!map_filled(tmp)){
        increment_key(tmp);
        ++size;
    }
    free(tmp);
    return size;
}
char *gen_word_key(struct dw_hashmap *map){
    /* Initialise key if needed */
    char **key = &map->gen_key;
    if ((*key) == NULL){
        init_key(key);
    } else {
        increment_key(*key);
    }
    return (*key);
}
bool map_insert(struct dw_hashmap *map, char *word){
    int pos = str_hash(word, CONFIG.map_size);
    bool uniq = node_unique(map->map[pos], word);
    if (!CONFIG.unique || uniq){
        /* Create node and insert if word is unique
         * or dw configured to not care about that */
        char *given_key = gen_word_key(map);
        struct dw_node *new = malloc_assert(sizeof(struct dw_node));
        new->key = pos;
        new->value.id = str_malloc(CONFIG.key_length);
        strcpy(new->value.id, given_key);
        new->value.value = str_malloc(strlen(word));
        strcpy(new->value.value, word);
        new->next = NULL;
        node_insert(&map->map[pos], new);
    }

    return uniq;
}
bool node_unique(struct dw_node *node, char *str){
    /* Follow linked list if word not matched,
     * NULL means unique to that specific list */
    if (node == NULL){
        return true;
    } 
    if (strcmp(node->value.value, str) == 0){
        return false;
    }
    return node_unique(node->next, str);
}
void node_insert(struct dw_node **node, struct dw_node *new_node){
    /* Get to end of list and insert */
    /* TODO: Insert at start instead of end */
    if (*node == NULL){
        *node = new_node;
    } else {
        node_insert(&(*node)->next, new_node);
    }
}
char *node_lookup(struct dw_node *node, char *id){
    /* Move through list until id is found, return empty string if not found */
    if (node == NULL){
        return "";
    }
    if (strcmp(node->value.id, id) == 0){
        return node->value.value;
    }
    return node_lookup(node->next, id);
}
char *map_lookup(struct dw_hashmap *map, char *id){
    char *ret = node_lookup(map->map[str_hash(id, CONFIG.map_size)], id);
    if (strcmp(ret, "") == 0){
        error("Could not find '%s' in hash %d\n", id, str_hash(id, CONFIG.map_size));
    }
    return ret;
}
void node_free(struct dw_node* node){
    /* Move to end of list and start freeing */
    if (node == NULL){
        return;
    }
    node_free(node->next);

    free(node->value.id);
    free(node->value.value);
    free(node->next);
}
void map_free(struct dw_hashmap* map){
    /* Free each linked list in map */
    if (map->map != NULL){
        for (int i = 0; i < CONFIG.map_size; ++i){
            if (map->map[i] != NULL){
                node_free(map->map[i]);
                free(map->map[i]);
            }
        }
    }

    free(map->map);
    free(map->gen_key);
    free(map);
}
void node_write(FILE *fp, struct dw_node *node){
    if (node == NULL){
        return;
    }
    fprintf(fp, "%s %s\n",node->value.id, node->value.value);
    node_write(fp, node->next);
}
void map_write(FILE *fp, struct dw_hashmap *map){
    /* Write list information, then all nodes */
    fprintf(fp, "%zu-%zu\n", (size_t)CONFIG.key_length, CONFIG.char_set_size);
    fprintf(fp, "%s\n", CONFIG.char_set);
    for (int i = 0; i < CONFIG.map_size; ++i){
        node_write(fp, map->map[i]);
    }
}
void node_rearrange(struct dw_node **map, struct dw_node *node){
    /* Generate and import only knows keys and uses hashed keys,
     * move to end, change hash to hashed key instead of word, then insert */
    if (node == NULL){
        return;
    }
    node_rearrange(map, node->next);
    node->next = NULL;
    node->key = str_hash(node->value.id, CONFIG.map_size);
    node_insert(&map[node->key], node);
}
void map_rearrange(struct dw_hashmap *map){
    /* Create new map to insert nodes to */
    struct dw_node **new_map = calloc_assert(CONFIG.map_size, sizeof(struct dw_node*));
    for (int i = 0; i < CONFIG.map_size; ++i){
        if (map->map[i] != NULL){
            node_rearrange(new_map, map->map[i]);
        }
    }
    /* Replace old, empty map with new map */
    free(map->map);
    map->map = new_map;
}

/* 
 * Mainly used for debugging purposes, may prove useful 
 * for other things however 
 */
int node_count(struct dw_node *node){
    int ret = 0;
    if (node != NULL){
        ret = 1;
        ret += node_count(node->next);
    }
    return ret;
}
void node_print(struct dw_node *node){
    if (node == NULL){
        printf("[NULL]\n");
        return;
    }
    printf("[%zu:[%s:%s]]->", node->key, node->value.id, node->value.value);
    node_print(node->next);
}
void map_print(struct dw_hashmap *map){
    for (int i = 0; i < CONFIG.map_size; ++i){
        if(map->map[i] != NULL){
            printf("[%d]->", i);
            printf("%d entries\n", node_count(map->map[i]));
        }
    }
}
