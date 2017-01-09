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
    char *gen_key;
};


bool map_filled(char*);
size_t map_left(char*);

void init_key(char**);
char *gen_word_key(struct dw_hashmap*);

int str_hash(char*,int);

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
