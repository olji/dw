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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <libconfig.h>

#include "config.h"
#include "mem.h"
#include "ioput.h"

#define DEF_LIST "default"
#define DEF_CHARSET "123456"

extern struct dw_config CONFIG;

char *unique(char *);
void rightshift_to(char*, int, int);
char *expand_string(const char*);
int write_config();
int read_config(char *configpath){
    /* Set default values */
    CONFIG.default_list = str_malloc(strlen(DEF_LIST));
    strcpy(CONFIG.default_list, DEF_LIST);
    CONFIG.key_length = 5;
    CONFIG.char_set = str_malloc(strlen(DEF_CHARSET));
    strcpy(CONFIG.char_set, DEF_CHARSET);
    CONFIG.unique = 1;
    CONFIG.script_friendly = 0;
    CONFIG.word_min_len = 2;
    CONFIG.gpg_key_id = "";
    CONFIG.use_gpg = 0;

    /* libconfig setup */
    config_t cfg;
    config_init(&cfg);

    /* If failing to read file, see if opening file fails and offer to create a default configuration if so */
    if (!config_read_file(&cfg, configpath)){
        bool exit = false;
        FILE *fp = fopen(configpath, "r");
        if (fp == NULL){
            note("Failed to open file '%s' for reading. ", configpath);
            char ans = ask("yn", "Create default config at '%s'?", configpath);
            if (ans == 'y'){
                if (write_config(&cfg, configpath) < 0){
                    return -1;
                }
            }
            ans = ask("yn", "Continue?");
            if (ans == 'n'){
                exit = true;
            }
        } else {
            error("%s\n%s\n", config_error_text(&cfg), config_error_file(&cfg));
            exit = true;
        }
        if (exit){
            config_destroy(&cfg);
            return -1;
        }
    }

    /* Most of this is self explanatory, read value using config_lookup_x(),
     * free any strings which should already be allocated with default value */
    const char *charset_string;
    const char *def_list_string;
    const char *gpg_key_id;
    if (config_lookup_string(&cfg, "default-list", &def_list_string)){
        free(CONFIG.default_list);
        CONFIG.default_list = str_malloc(strlen(def_list_string));
        strcpy(CONFIG.default_list, def_list_string);
    }
    if (config_lookup_string(&cfg, "gpg-key", &gpg_key_id)){
        CONFIG.gpg_key_id = str_malloc(strlen(gpg_key_id));
        strcpy(CONFIG.gpg_key_id, gpg_key_id);
    }
    if (config_lookup_string(&cfg, "character-set", &charset_string)){
        char *full_character_set = expand_string(charset_string);

        sort(full_character_set);
        full_character_set = unique(full_character_set);

        free(CONFIG.char_set);
        CONFIG.char_set = full_character_set;
        debug("Charset: %s\n", CONFIG.char_set);
    }
    config_lookup_int(&cfg, "key-length", &CONFIG.key_length);
    config_lookup_int(&cfg, "word-min-length", &CONFIG.word_min_len);
    config_lookup_bool(&cfg, "unique", &CONFIG.unique);
    config_lookup_bool(&cfg, "script-friendly", &CONFIG.script_friendly);
    config_lookup_bool(&cfg, "use-gpg", &CONFIG.use_gpg);

    CONFIG.char_set_size = strlen(CONFIG.char_set);
    CONFIG.map_size = pow(CONFIG.char_set_size, CONFIG.key_length);

    config_destroy(&cfg);
    return true;
}

char *expand_string(const char* charset_string){
    char start_char;
    char end_char;
    /* full_group will be the character set with all eventual groups expanded */
    char *full_character_set = calloc_assert(1, sizeof(char));
    /* group_part is the fully expanded string from e.g. [az09] */
    char *full_group;
    int str_part_start = 0;
    enum parse_mode {NORMAL = 0, CHAR_GROUP} p_mode = NORMAL;
    for (int i = 0; i <= strlen(charset_string); ++i){
        /* If end of string reached, copy uncollected characters */
        if (charset_string[i] == '\0' && str_part_start < i){
            char *tmp = str_malloc(strlen(full_character_set) + (i - str_part_start));
            strcpy(tmp, full_character_set);
            strcat(tmp, &charset_string[str_part_start]);
            free(full_character_set);
            full_character_set = tmp;
        }

        switch (p_mode){
            case NORMAL:
                /* If unescaped opening square bracket... */
                if (charset_string[i] == '['){
                    if (i == 0 || (charset_string[i-1] != '\\' &&
                                strstr(charset_string, "]"))){
                        /* Copy the string before the character '[' into full group */
                        if (i != str_part_start){
                            char *tmp;
                            tmp = str_malloc(strlen(full_character_set) + (i - str_part_start) - 1);
                            strcpy(tmp, full_character_set);
                            strncat(tmp, charset_string, i - (str_part_start));
                            free(full_character_set);
                            full_character_set = tmp;
                        }
                        full_group = calloc_assert(1, sizeof(char));
                        p_mode = CHAR_GROUP;
                        break;
                    }
                }
                break;
                /* Inside a square bracket statement */
            case CHAR_GROUP:
                if (charset_string[i] == ']' &&
                        charset_string[i - 1] != '\\'){
                    /* Append all expanded groups stored in full_group into full_character_set */
                    char *tmp = str_malloc(strlen(full_group) + strlen(full_character_set) + 1);
                    strcpy(tmp, full_character_set);
                    strcat(tmp, full_group);
                    free(full_group);
                    free(full_character_set);
                    full_character_set = tmp;
                    /* Set str_part_start to after closing square bracket */
                    str_part_start = i+1;
                    p_mode = NORMAL;
                    break;
                }
                start_char = charset_string[i++];
                end_char = charset_string[i];
                /* Swap if start_char appears after end_char in ascii table */
                if (start_char > end_char){
                    char tmp = start_char;
                    start_char = end_char;
                    end_char = tmp;
                }
                char *group_part = str_malloc(abs(end_char - start_char) + 1);
                /* Terminate string */
                group_part[end_char - start_char + 1] = '\0';
                /* Add characters in ascii table from start_char to end_char */
                for (int i = 0; start_char <= end_char; ++i, ++start_char){
                    group_part[i] = start_char;
                }
                /* Append group_part into full_group */
                char *tmp = str_malloc(strlen(group_part) + strlen(full_group));
                strcpy(tmp, full_group);
                strcat(tmp, group_part);
                free(group_part);
                free(full_group);
                full_group = tmp;
        }
    }
    return full_character_set;
}
/* Simple insertion sort */
void sort(char *str){
    for (int i = 0, j = i; i < strlen(str); ++i, j = i){
        while (j > 0 && str[j-1] > str[j]){
            char t = str[j-1];
            str[j-1] = str[j];
            str[j] = t;
            --j;
        }
    }
}
char *unique(char *str){
    /* Shift non-duplicates forward, leaving trash at the beginning of the string */
    int duplicates = 0;
    for (int i = 0; i < strlen(str); ++i){
        if (str[i] == str[i + 1]){
            rightshift_to(str, duplicates++, i);
        }
    }
    char *new = str_malloc(strlen(str) - duplicates);
    strcpy(new, &str[duplicates]);
    free(str);
    if (duplicates > 0){
        note("%d duplicate characters removed from expanded character set string\n", duplicates);
    }
    return new;
}
void rightshift_to(char *str, int start, int end){
    if (end <= start){
        return;
    }
    str[end] = str[end-1];
    rightshift_to(str, start, end-1);
}
int write_config(config_t *cfg, char *configpath){
    /* Setup default configuration for libconfig */
    config_setting_t *root, *setting;
    root = config_root_setting(cfg);
    /* Set default-list */
    setting = config_setting_add(root, "default-list", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, CONFIG.default_list);
    /* Set key-length */
    setting = config_setting_add(root, "key-length", CONFIG_TYPE_INT);
    config_setting_set_int(setting, CONFIG.key_length);
    /* Set character-set */
    setting = config_setting_add(root, "character-set", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, CONFIG.char_set);
    /* Set unique */
    setting = config_setting_add(root, "unique", CONFIG_TYPE_BOOL);
    config_setting_set_bool(setting, CONFIG.unique);
    /* Set script-friendly */
    setting = config_setting_add(root, "script-friendly", CONFIG_TYPE_BOOL);
    config_setting_set_bool(setting, CONFIG.script_friendly);
    /* Set word-min-length */
    setting = config_setting_add(root, "word-min-length", CONFIG_TYPE_INT);
    config_setting_set_int(setting, CONFIG.word_min_len);
    /* Set use-gpg */
    setting = config_setting_add(root, "use-gpg", CONFIG_TYPE_BOOL);
    config_setting_set_bool(setting, CONFIG.use_gpg);
    /* Set gpg-key */
    setting = config_setting_add(root, "gpg-key", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, CONFIG.gpg_key_id);
    if (!config_write_file(cfg, configpath)){
        error("Could not write default configuration to file '%s'", configpath);
        config_destroy(cfg);
        return -1;
    }
    return 0;
}
void conf_free(){
    free(CONFIG.default_list);
    free(CONFIG.char_set);
}
