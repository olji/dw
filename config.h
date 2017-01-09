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
 * along with dw.  If not, see <http://www.gnu.org/licenses/>.#include <stdio.h>
 */
#ifndef CONFIG_H
#define CONFIG_H

struct dw_config{
    char *default_list;
    int key_length;
    char *char_set;
    size_t char_set_size;
    size_t map_size;
    int unique;
    int script_friendly;
    int word_min_len;
} CONFIG;

int read_config(char*);
void conf_free();
void sort(char *);

#endif
