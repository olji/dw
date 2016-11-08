#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "map.h"
#include "config.h"

extern struct dw_config CONFIG;

static char *key;
int str_hash(char *word, int modval){
    size_t pos = 0;
    for (int i=0; word[i] != '\0'; ++i){
        word[i] = tolower(word[i]);
        pos += word[i];
        pos = pos - pos + pow(pos, 2);
    }
    return pos % modval;
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
bool map_insert(struct dw_hashmap *map, char *word){
    int pos = str_hash(word, CONFIG.map_size);
    bool uniq = node_unique(map->map[pos], word);
    if (!CONFIG.unique || uniq){
        char *given_key = gen_word_key();
        struct dw_node *new = malloc(sizeof(struct dw_node));
        new->key = pos;
        new->value.id = malloc(sizeof(char) * (CONFIG.key_length + 1));
        strcpy(new->value.id, given_key);
        new->value.value = malloc(sizeof(char) * (strlen(word) + 1));
        strcpy(new->value.value, word);
        new->next = NULL;
        node_insert(&map->map[pos], new);
    }

    return uniq;
}
bool node_unique(struct dw_node *node, char *str){
    if (node == NULL){
        return true;
    } 
    if (strcmp(node->value.value, str) == 0){
        return false;
    }
    return node_unique(node->next, str);
}
void node_insert(struct dw_node **node, struct dw_node *new_node){
    if (*node == NULL){
        *node = new_node;
    } else {
        node_insert(&(*node)->next, new_node);
    }
}
char *node_lookup(struct dw_node *node, char *id){
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
        printf("ERR: Could not find '%s' in hash %d\n", id, str_hash(id, CONFIG.map_size));
    }
    return ret;
}
void node_delete(struct dw_node* node){
    if (node->next != NULL){
        node_delete(node->next);
    }
    free(node->value.id);
    free(node->value.value);
    free(node->next);
}
void map_delete(struct dw_hashmap* map){
    for (int i = 0; i < CONFIG.map_size; ++i){
        if (map->map[i] != NULL){
            node_delete(map->map[i]);
            free(map->map[i]);
        }
    }
    free(map->map);
    free(map);
}
void node_write(FILE *fp, struct dw_node *node){
    if (node == NULL){
        return;
    }
    fprintf(fp, "%s\n%zu\n%s\n",node->value.id, strlen(node->value.value), node->value.value);
    node_write(fp, node->next);
}
void map_write(FILE *fp, struct dw_hashmap *map){
    fprintf(fp, "%zu-%zu\n", (size_t)CONFIG.key_length, CONFIG.char_set_size);
    fprintf(fp, "%s\n", CONFIG.char_set);
    for (int i = 0; i < CONFIG.map_size; ++i){
        node_write(fp, map->map[i]);
    }
}
void  map_rearrange(struct dw_hashmap *map){
    for (int i = 0; i < CONFIG.map_size; ++i){
        if (map->map[i] != NULL){
        }
    }
}

/* 
 * Mainly used for debugging purposes, may prove useful 
 * for other things however 
 */
void node_print(struct dw_node *node){
    if (node == NULL){
        printf("[NULL]\n");
        return;
    }
    printf("[%d:[%s:%s]]->", node->key, node->value.id, node->value.value);
    node_print(node->next);
}
void map_print(struct dw_hashmap *map){
    for (int i = 0; i < CONFIG.map_size; ++i){
        printf("[%d]->", i);
        node_print(map->map[i]);
    }
}
