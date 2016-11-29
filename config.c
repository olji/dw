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
#include <confuse.h>

#include "config.h"

#define DEF_LIST "default"
#define DEF_CHARSET "0123456789"

extern struct dw_config CONFIG;

int read_config(char *configpath){
    /* Set default values */
    CONFIG.default_list = malloc(sizeof(char) * (strlen(DEF_LIST) + 1));
    strcpy(CONFIG.default_list, DEF_LIST);
    CONFIG.key_length = 5;
    CONFIG.char_set = malloc(sizeof(char) * (strlen(DEF_CHARSET) + 1));
    strcpy(CONFIG.char_set, DEF_CHARSET);
    CONFIG.unique = true;
    CONFIG.word_min_len = 2;

    FILE *cf = fopen(configpath, "r");
    if (cf == NULL){
        printf("Could not open %s, create default configuration file? [y/n]: ", configpath);
        char ans = 0;
        do{
            if (ans != 0){
                printf("Please answer y or n\n");
            }
            scanf("%s", &ans);
        } while (ans != 'y' && ans != 'n');
        if (tolower(ans) == 'y'){
            if (!write_default_config(configpath)){
                return -1;
            }
        }
        printf("Continue? [y/n] (or exit): ");

        ans = 0;
        do{
            if (ans != 0){
                printf("Please answer y or n\n");
            }
            scanf("%s", &ans);
        } while (ans != 'y' && ans != 'n');
        return (tolower(ans) == 'y') ? true : false;
    }
    fclose(cf);

    cfg_t *cfg;
    cfg_opt_t opts[] = {
        CFG_SIMPLE_BOOL("unique-words", &CONFIG.unique),
        CFG_SIMPLE_INT("min-word-length", &CONFIG.word_min_len),
        CFG_SIMPLE_INT("key-length", &CONFIG.key_length),
        CFG_SIMPLE_STR("default-list", &CONFIG.default_list),
        CFG_SIMPLE_STR("character-set", &CONFIG.char_set),
        CFG_END()
    };

    cfg = cfg_init(opts, CFGF_NOCASE);
    cfg_parse(cfg, configpath);

    /*
     * Parse character groups in character set such as [09][az] or similar.
     * TODO: Error checking for unterminated character groups.
     * TODO: Ability to define several groups inside same square brackets, e.g. [az09]
     */
    char start_char;
    char end_char;
    char *full_group;
    int group_start = 0;
    enum parse_mode {NORMAL = 0, CHAR_GROUP} p_mode = NORMAL;
    for (int i = 0; i < strlen(CONFIG.char_set); ++i){
        switch (p_mode){
        case NORMAL:
            if (CONFIG.char_set[i] == '['){
                if (i == 0 || (CONFIG.char_set[i-1] != '\\' &&
                    strstr(CONFIG.char_set, "]"))){
                    group_start = i;
                    p_mode = CHAR_GROUP;
                    break;
                }
            }
            break;
        case CHAR_GROUP:
            if (CONFIG.char_set[i] == ']'){
                if (CONFIG.char_set[i-1] != '\\'){
                    /* Replace [group] with variable full_group */
                    char *end_string = calloc(strlen(&CONFIG.char_set[i]), sizeof(char));
                    strcpy(end_string, &CONFIG.char_set[i+1]);
                    char *new_string = calloc(group_start + strlen(end_string) + strlen(full_group) + 1, sizeof(char));
                    strncpy(new_string, CONFIG.char_set, group_start);
                    strcat(new_string, full_group);
                    strcat(new_string, end_string);
                    free(CONFIG.char_set);
                    CONFIG.char_set = new_string;

                    /* Exit point of CHAR_GROUP (Unless unterminated character group */
                    free(end_string);
                    free(full_group);

                    p_mode = NORMAL;
                    break;
                }
            }
            start_char = CONFIG.char_set[i++];
            end_char = CONFIG.char_set[i];
            /* Switch character order if start_char appears after end_char in ascii table */
            if (start_char > end_char){
                char tmp = start_char;
                start_char = end_char;
                end_char = tmp;
            }
            full_group = malloc(sizeof(char) * (abs(end_char - start_char) + 2));
            full_group[end_char - start_char + 1] = '\0';
            for (int i = 0; start_char <= end_char; ++i, ++start_char){
                full_group[i] = start_char;
            }
        }
    }

    CONFIG.char_set_size = strlen(CONFIG.char_set);
    CONFIG.map_size = pow(CONFIG.char_set_size, CONFIG.key_length);

    cfg_free(cfg);
    free(configpath);
    return true;
}
bool write_default_config(char *configpath){
    FILE *cf = fopen(configpath, "wx");
    if (cf == NULL){
        printf("Could not open/create file %s for writing\n", configpath);
    }
    fprintf(cf, "# The list used if omitted on command line\ndefault-list = \"%s\"\n# Character set used for word keys of diceware lists created from using -c, character groups can be specified, e.g. [09] will result in a string of every character between the characters, including the start and end points\ncharacter-set = \"%s\"\n# Length of key for every word during creation of diceware lists by using -c\nkey-length = %ld\n# If true, enforces the need of unique words in the diceware list, \n# HIGHLY RECOMMENDED to leave true as duplicate words weaken the security of generated passphrases\nunique-words = %s\n# Minimum word length of each word in the diceware list, \n# it is recommended to have a length of at least 14 characters, \n# including spaces, for a passphrase of 5 words and 17 characters for \n# 6 words, making a minimum word length of 2 ensure every passphrase \n# should follow that length, if set to 1 it is recommended to manually \n# ensure that the passphrase keeps a good length (Average of two characters per word or higher).\nmin-word-length = %ld",CONFIG.default_list, CONFIG.char_set, CONFIG.key_length, (CONFIG.unique ? "true": "false"), CONFIG.word_min_len);
    fclose(cf);
    return true;
}
void conf_free(){
    free(CONFIG.default_list);
    free(CONFIG.char_set);
}
