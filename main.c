/*
 * dw - A simple diceware manager
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
/* TODO: Add optional gpgme support */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <argp.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "map.h"
#include "config.h"
#include "ioput.h"
#include "mem.h"

#define DEF_HOME "/.dw/"
#define BUF_SIZE 128

extern struct dw_config CONFIG;

const char *argp_program_version = "dw 1.1";
const char *argp_program_bug_address = "me@rickardjonsson.se";

static char doc[] = "dw - Diceware manager";

static char args_doc[] = "[LIST]";

/* Linked list structure for keeping track of several option arguments */
struct arg_pair {
    enum {NONE = 0, INPUT_FILE, PP_LENGTH} type;
    char *value;
    struct arg_pair *next;
};
struct arguments {
    enum {LIST_NONE = 0, CREATE, IMPORT} list_option;
    enum {DW_NONE = 0, GEN, LOOK} dw_option;
    bool ext_list;
    bool ext_cfg;
    char *list;
    char *cfg;
    struct arg_pair *arguments;
};

void generate(struct dw_hashmap*, int);
void lookup(FILE*, struct dw_hashmap*);
bool arguments_insert(struct arguments*, int, char*);
/* Returns <0 if error, 0 if unsuccessful but nothing fatal, 1 if successful */
int list_create(FILE*, struct dw_hashmap*);
bool list_import(FILE*, struct dw_hashmap*);
bool list_parse(FILE*, struct dw_hashmap*);
char **parse_key_word(char*, char*);
void args_free(struct arg_pair*);

static error_t parse_opt (int key, char *arg, struct argp_state *state){
    /* TODO: Error message if conflicting options are chosen (Such as both --generate and --lookup */
    struct arguments *arguments = state->input;
    switch (key){
    case 'g':
        arguments->dw_option = GEN;
        if (arg != NULL){
            /* Make sure only integers */
            for (int i = 0; i < strlen(arg); ++i){
                if (!isdigit(arg[i])){
                    error("Optional parameter to -g (--generate) should only be integers\n");
                    exit(1);
                }
            }
            arguments_insert(arguments, PP_LENGTH, arg);
        } else {
        }
        break;
    case 'l':
        arguments->dw_option = LOOK;
        break;
    case 'c':
        arguments->list_option = CREATE;
        debug("Input file: %s\n", arg);
        arguments_insert(arguments, INPUT_FILE, arg);
        break;
    case 'i':
        arguments->list_option = IMPORT;
        debug("Input file: %s\n", arg);
        arguments_insert(arguments, INPUT_FILE, arg);
        break;
    case 'u':
        arguments->list = str_malloc(strlen(arg));
        strcpy(arguments->list, arg);
        arguments->ext_list = true;
        break;
    case 'U':
        arguments->cfg = str_malloc(strlen(arg));
        strcpy(arguments->cfg, arg);
        arguments->ext_cfg = true;
        break;
    case ARGP_KEY_ARG:
        /* Check for going upwards in directories or starting from root */
        if (strstr(arg, "..") == NULL && arg[0] != '/'){
            arguments->list = arg;
        } else {
            error("List files outside of dw home directory should be supplied with -u\n");
            exit(1);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp_option options[] = {
    { "generate", 'g', "WORD_COUNT", OPTION_ARG_OPTIONAL, "Generate a passphrase using LIST", 1 },
    { "lookup", 'l', "PW_FILE", OPTION_ARG_OPTIONAL, "Look up passphrase using LIST", 1 },
    { "import-list", 'i', "FILE", 0, "Import diceware list named LIST", 0 },
    { "create-list", 'c', "FILE", 0, "Create a new diceware list named LIST", 0 },
    { "use-list", 'u', "LIST", 0, "Use given LIST not present in home directory of dw", 0 },
    { "use-config", 'U', "CONFIG", 0, "Use given CONFIG not necessarily present in home directory of dw", 0 },
    { 0 }
};

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv){
    /* TODO: Cleaner way of cleanup (Wrapper to free to set pointer to null if freed, and not try to free if null?) */
#if DEBUG == 1
    printf("DEBUG IS ON\n===================\n");
#endif
    if (argc < 2){
        printf("No function asked for.\n\n");
        argp_help(&argp, stdout, ARGP_HELP_SHORT_USAGE, "dw");
        argp_help(&argp, stdout, ARGP_HELP_LONG, "dw");
        return 0;
    }

    struct arguments input_args = {
        LIST_NONE, /* list_option */
        DW_NONE, /* dw_option */
        false, /* ext_list */
        false, /* ext_cfg */
        NULL, /* list_file */
        "dw.conf", /* cfg_file */
        NULL, /* arguments */
    };
    argp_parse (&argp, argc, argv, 0, 0, &input_args);
    char *home = NULL;
    {
        /* Set home to DW_HOME if set, otherwise default */
        char *dw_home = getenv("DW_HOME");
        if (dw_home == NULL){
            note("DW_HOME is not set, assuming ~/.dw/\n");
            char *usr_home = getenv("HOME");
            home = str_malloc((strlen(usr_home) + strlen(DEF_HOME)));
            strcpy(home, usr_home);
            strcat(home, DEF_HOME);
        } else {
            home = str_malloc(strlen(dw_home));
            strcpy(home, dw_home);
        }
    }

    /* Set config path according to home, or value supplied by option -U */
    char *cfg_path;
    if (input_args.ext_cfg){
        cfg_path = input_args.cfg;
    } else {
        cfg_path = str_malloc(strlen(home) + strlen(input_args.cfg));
        strcpy(cfg_path, home);
        strcat(cfg_path, input_args.cfg);
    }
    debug("cfg_path: %s\n", cfg_path);

    int exit_status = read_config(cfg_path);
    /*
     * Will free input_args.cfg if defined as well,
     * no need to free in cleanup
     */
    free(cfg_path);
    if (exit_status < 1){
        conf_free();
        args_free(input_args.arguments);
        free(home);
        if (input_args.ext_list){
            free(input_args.list);
        }
        return abs(exit_status);
    }

    if (input_args.list == NULL){
        /* Set default list if possible or exit */
        printf("Using default list...\n");
        if (CONFIG.default_list != NULL){
            input_args.list = str_malloc(strlen(CONFIG.default_list));
            strcpy(input_args.list, CONFIG.default_list);
        } else {
            error("default list not set, set default list in configuration or provide list name manually\n");
            conf_free();
            args_free(input_args.arguments);
            free(home);
            if (input_args.ext_list){
                free(input_args.list);
            }
            return 1;
        }
    }

    debug("List file used: %s\n", input_args.list);
    char *listpath;
    /* Set list path */
    if (input_args.ext_list){
        listpath = str_malloc(strlen(input_args.list));
        strcpy(listpath, input_args.list);
    } else {
        listpath = str_malloc(strlen(home) + strlen(input_args.list));
        strcpy(listpath, home);
        strcat(listpath, input_args.list);
    }
    debug("Listpath: %s\n", listpath);

    char *input_file;
    int g_length = 0;

    {
        struct arg_pair *node = input_args.arguments;
        /* Parse eventual option arguments passed */
        while (node != NULL){
            if (node->type == INPUT_FILE){
                input_file = node->value;
            } else if (node->type == PP_LENGTH){
                char* end;
                g_length = (int)strtol(node->value, &end, 10);
            }
            node = node->next;
        }
    }
    FILE *list;

    /*
     * We currently cannot be sure of the actual size of the pointer array,
     * so we wait with allocating memory for that, but set it to NULL.
     */
    struct dw_hashmap *dw_list = malloc_assert(sizeof(struct dw_hashmap));
    dw_list->map = NULL;
    /* Gen key will be allocated upon first call to map_insert() */
    dw_list->gen_key = NULL;
    if (input_args.list_option != LIST_NONE){
        /* Necessary preparation for list creation options (--create, --import) */

        /* Does file open? Does it exist? */
        list = fopen(listpath, "r");
        if (list != NULL){
            fclose(list);
            if (input_args.ext_list){
                /* Don't prompt for removal outside home, files will not necessarily "belong" to dw */
                error("File already exists, will not delete files outside %s\n", home);
                conf_free();
                args_free(input_args.arguments);
                free(home);
                free(listpath);
                map_free(dw_list);
                if (input_args.ext_list){
                    free(input_args.list);
                }
                return 1;
            }
            /* If in home, chances are dw created them. (Malicious removal using dw is just stupid, works, but stupid) */
            char ans = ask("yn", "File already exists: %s, delete?", listpath);
            if (ans == 'y'){
                remove(listpath);
            } else {
                conf_free();
                args_free(input_args.arguments);
                free(home);
                free(listpath);
                map_free(dw_list);
                if (input_args.ext_list){
                    free(input_args.list);
                }
                return 1;
            }
        }

        FILE *input = fopen(input_file, "r");
        if (input == NULL){
            error("Could not open input file %s\n", input_file);
            conf_free();
            args_free(input_args.arguments);
            free(home);
            free(listpath);
            map_free(dw_list);
            if (input_args.ext_list){
                free(input_args.list);
            }
            return 1;
        }
        /* Perform chosen option */
        switch (input_args.list_option){
        case CREATE:
            exit_status = list_create(input, dw_list);
            if (exit_status < 1){
                conf_free();
                args_free(input_args.arguments);
                free(home);
                free(listpath);
                map_free(dw_list);
                if (input_args.ext_list){
                    free(input_args.list);
                }
                return abs(exit_status);
            }
            break;
        case IMPORT:
            if (!list_import(input, dw_list)){
                conf_free();
                args_free(input_args.arguments);
                free(home);
                free(listpath);
                map_free(dw_list);
                if (input_args.ext_list){
                    free(input_args.list);
                }
                return 1;
            }
            break;
        default:
            printf("list_option default case reached, exiting.\n");
            conf_free();
            args_free(input_args.arguments);
            free(home);
            free(listpath);
            map_free(dw_list);
            if (input_args.ext_list){
                free(input_args.list);
            }
            return 2;
        }
        fclose(input);
        /* Write list to file */
        list = fopen(listpath, "wx");
        map_write(list, dw_list);
        fclose(list);
    }

    if (input_args.dw_option != DW_NONE){
        if (input_args.list_option == LIST_NONE){
            /* If --create nor --import has been used, dw_list will be free */
            list = fopen(listpath, "r");
            if (list == NULL){
                error("Failed to open list %s for reading.\n", listpath);
                conf_free();
                args_free(input_args.arguments);
                free(home);
                free(listpath);
                map_free(dw_list);
                if (input_args.ext_list){
                    free(input_args.list);
                }
                return 1;
            }
            debug("Opened: %s\n", listpath);
            bool ret = list_parse(list, dw_list);
            fclose(list);
            if (!ret){
                printf("Exiting...\n");
                free(home);
                free(listpath);
                map_free(dw_list);
                args_free(input_args.arguments);
                conf_free();
                if (input_args.ext_list){
                    free(input_args.list);
                }
                return 1;
            }
        } else {
            /* If they have been used the list is already in memory, just some rearrangements needed */
            map_rearrange(dw_list);
        }

        /* Call function asked for */
        switch (input_args.dw_option){
        case GEN:
            generate(dw_list, g_length);
            break;
        case LOOK:
            /* TODO: Pass file pointer if file given */
            lookup(NULL, dw_list);
            break;
        default:
            printf("dw_option default case reached, exiting.");
            conf_free();
            args_free(input_args.arguments);
            free(home);
            free(listpath);
            map_free(dw_list);
            if (input_args.ext_list){
                free(input_args.list);
            }
            return 2;
        }
    }

    if (input_args.ext_list){
        free(input_args.list);
    }
    free(home);
    free(listpath);
    map_free(dw_list);
    args_free(input_args.arguments);
    conf_free();
    return 0;
}

void generate(struct dw_hashmap *dw_list, int length){

    if (length == 0){
        do{
            printf("How many words? ");
            scanf("%d", &length);
        } while (length <= 0);
    }
    /* One key */
    char *id = str_malloc(CONFIG.key_length);
    srand(time(NULL));
    /* Passphrase itself */
    char *passphrase = str_malloc(0);
    /* Whole string of keys
     * Calloc pw_id for simpler concatenation inside loop */
    char *pw_id = calloc_assert((length*CONFIG.key_length) + length + 1, sizeof(char));

    for (int i = 0; i < length; ++i){
        /* Add random character to id n times, where n is length of id*/
        for (int j = 0; j < CONFIG.key_length; ++j){
            id[j] = CONFIG.char_set[rand()%CONFIG.char_set_size];
        }
        strcat(pw_id, id);
        strcat(pw_id, " ");

        char *found_word = map_lookup(dw_list, id);

        /* Add byte for space character */
        char *tmp_pw = str_malloc(strlen(passphrase) + strlen(found_word) + 1);
        /* Append new word to passphrase */
        strcpy(tmp_pw, passphrase);
        if (tmp_pw[0] != '\0'){
            strcat(tmp_pw, " ");
        }
        strcat(tmp_pw, found_word);
        free(passphrase);
        passphrase = tmp_pw;
    }
    printf("Passphrase: %s\n", passphrase);
    printf("Passphrase ID: %s\n", pw_id);
    free(id);
    free(pw_id);
    free(passphrase);
}
void lookup(FILE *file, struct dw_hashmap *dw_list){
    if (CONFIG.script_friendly == false){
        printf("Enter diceware passkeys (Ctrl+d to exit)\n");
    }
    if (file == NULL){
        file = stdin;
    }
    char c;
    char *key = str_malloc(0);
    /* Read input character by character */
    while (!feof(file)){
        c = fgetc(file);
        if (feof(file)){
            break;
        }

        /* Add to key if character exist in character set */
        if (strchr(CONFIG.char_set, c) != NULL){
            str_append(&key, c);
        }
        /* If blank or newline, verify key length and lookup word to print */
        else if (c == '\n' || isblank(c) != 0){
            if (strlen(key) == CONFIG.key_length){
                char *word = map_lookup(dw_list, key);
                printf("%s", word);
                if (isblank(c) != 0){
                    printf(" ");
                } else {
                    printf("\n");
                }
            } else {
                error("Length of %s is %zu, length of keys in list are %d.\n", key, strlen(key), CONFIG.key_length);
            }
            free(key);
            key = str_malloc(0);

        } else {
            /* If unsupported character (Not defined in character set),
             * read until blank or newline to remove rest of unusable key */
            do{
                str_append(&key, c);
                c = fgetc(file);
            } while (c != '\n' && isblank(c) == 0);
            error("Key %s is not valid in current list.\n", key);
            free(key);
            key = str_malloc(0);
        }
    }
}
int list_create(FILE *input_file, struct dw_hashmap *dw_list){
    if (!CONFIG.unique){
        note("dw is currently set to NOT enforce unique words in the diceware list, which may result in duplicate entries, ");
        char ans = ask("yn", "really continue?");
        if (ans == 'n'){
            return 0;
        }
    }
    FILE *file = input_file;
    if (file == NULL){
        file = stdin;
    }
    dw_list->map = calloc_assert(CONFIG.map_size, sizeof(struct dw_node*));

    char *delims = " \n";
    char chunk[BUF_SIZE];

    /* Mmmm... Ugly goto's for now */
L_CR_READ:
    /* BUF_SIZE needs to be same between to list creations if one want to recreate a list */
    while (fgets(chunk, BUF_SIZE, file) != NULL){
        char *str = strtok(chunk, delims);
        /* Add every supplied word to dw_list if equal to or larger than
         * CONFIG.word_min_len, exit prematurely if list is filled */
        while (str != NULL){
            if (strlen(str) >= CONFIG.word_min_len){
                map_insert(dw_list, str);
            }
            if (map_filled(dw_list->gen_key)){
                /* Hopefully this goto can be removed without too much screwing around */
                goto L_CR_READ_OUT;
            }
            str = strtok(NULL, delims);
        }
    }
L_CR_READ_OUT:
    /* Not being filled at this point means all input has been used, show
     * amount of words still needed and propose use of stdin instead */
    if (!map_filled(dw_list->gen_key) && file != stdin){
        note("Input file did not give sufficient amount of words to fill, %zu more words are needed. ", map_left(dw_list->gen_key));
        if (CONFIG.script_friendly == false){
            char ans = ask("yn","Want to use stdin to supply extra?");
            if (ans == 'y'){
                file = stdin;
                /* This should be removed later, it works for now, which is what's important at this point */
                goto L_CR_READ;
            }
        } else {
            file = stdin;
            /* Same like the goto above */
            goto L_CR_READ;
        }
    }
    /* Still not filled means use of stdin wasn't wanted, exit with error */
    if (!map_filled(dw_list->gen_key)){
        error("Could not gather %zu %swords from input file, lower key size, present a file with more %swords or reduce the size of your character set.%s\n", CONFIG.map_size, (CONFIG.unique?"unique ":""), (CONFIG.unique?"unique ":""), (CONFIG.unique?"\nEnabling duplicate words by setting unique_words to false in may result in gathering enough words, but WILL weaken the security of passphrases generated":""));
        return -1;
    }
    return 1;
}
bool list_import(FILE *input_file, struct dw_hashmap *dw_list){
    printf("Enter format of key-word pairs, k being key and w being word (E.g. k:w for key followed by word, separated by colon): ");
    char c = 0;
    char *format = str_malloc(0);
    /* Prompt for format */
    while (c != '\n' && c != EOF){
        c = fgetc(stdin);
        char *tmp = str_malloc(strlen(format) + 1);
        strcpy(tmp, format);
        tmp[strlen(format)] = c;
        free(format);
        format = tmp;
    }
    debug("format: %s\n", format);

    /* List should be imported, meaning stdin is not needed.
     * Thus read whole file */
    fseek(input_file, 0, SEEK_END);
    int f_size = ftell(input_file);
    rewind(input_file);

    char *chunk = str_malloc(f_size);
    fread(chunk, f_size, 1, input_file);

    char *string = NULL;
    char *char_set = str_malloc(0);
    size_t key_size = 0, map_size = 0, result_amount = 0;

    /* Currently works as specified below, but perhaps later rewritten to
     * a struct format for better readability overall */
    /* Level 0: Every key-word pair */
    /* Level 1: One specific key-word pair, 0 being key, 1 being word */
    /* Level 2: Value */
    char ***all_results = NULL;
    string = strtok(chunk, "\n");
    /* First pass, needed to ensure full character set */
    do{
        char **result = parse_key_word(format, string);
        /* Key_size should be the same, any changes means list is possibly faulty */
        if (key_size == 0){
            key_size = strlen(result[0]);
        } else if (key_size != strlen(result[0])){
            error("Key length not consistent!\n");
            return false;
        }

        /* Key was (presumably) proper, allocate space for another key-word pair */
        ++result_amount;
        char ***tmp = realloc(all_results, sizeof(char**) * result_amount);
        if (tmp == NULL){
            error("Error on reallocation!\n");
            return false;
        }
        all_results = tmp;
        all_results[result_amount-1] = result;

        /* For each character in key... */
        for(int i = 0; i < strlen(result[0]); ++i){
            /* Append character to character set if it is not found there */
            if (strchr(char_set, result[0][i]) == NULL){
                str_append(&char_set, result[0][i]);
            }
        }
    } while ((string = strtok(NULL, "\n")) != NULL);

    sort(char_set);
    map_size = pow(strlen(char_set), key_size);
    free(CONFIG.char_set);

    /* Set configuration, only one instance of dw_hashmap should exist,
     * but having separate members stored in the struct itself might be nicer */
    CONFIG.char_set = char_set;
    CONFIG.map_size = map_size;
    CONFIG.key_length = key_size;
    CONFIG.char_set_size = strlen(char_set);

    dw_list->map = calloc_assert(map_size, sizeof(struct dw_node*));

    for (int i = 0; i < result_amount; ++i){
        /* Convert char*** structure to dw_list */
        struct dw_node *new = malloc_assert(sizeof(struct dw_node));
        new->value.id = all_results[i][0];
        new->value.value = all_results[i][1];
        new->key = str_hash(all_results[i][0], map_size);
        new->next = NULL;
        node_insert(&(dw_list->map[new->key]), new);
        free(all_results[i]);
    }
    free(format);
    free(all_results);
    free(chunk);
    return true;
}
bool list_parse(FILE *list, struct dw_hashmap *dw_list){
    /* Get filesize */
    fseek(list, 0, SEEK_END);
    int f_size = ftell(list);
    rewind(list);

    /* Read full file */
    char *chunk = str_malloc(f_size);
    fread(chunk, f_size, 1, list);
    char *str = strtok(chunk, "\n");

    size_t key_size = 0, charset_size = 0;
    debug("chunk: %s, f_size: %d, chunk_part: %s\n", chunk, f_size, str);

    /* First line is keysize and character set size, separated with hyphen */
    sscanf(str, "%zu-%zu", &key_size, &charset_size);
    if (key_size == 0 || charset_size == 0){
        error("Missing information about key size or character set length\n");
        free(chunk);
        return false;
    }

    char *charset = str_malloc(charset_size);
    const int map_size = pow(charset_size, key_size);

    /* Second line is character set */
    str = strtok(NULL, "\n");
    sscanf(str, "%s", charset);

    if (strlen(charset) != charset_size){
        error("Given character set does not correspond to given length of character set, list possibly faulty\n");
        free(chunk);
        free(charset);
        return false;
    }

    free(CONFIG.char_set);

    /* Rest of the lines should all be list entries */

    /* Set configuration, only one instance of dw_hashmap should exist,
     * but having separate members stored in the struct itself might be nicer */
    CONFIG.key_length = key_size;
    CONFIG.char_set = str_malloc(strlen(charset));
    strcpy(CONFIG.char_set, charset);
    CONFIG.char_set_size = charset_size;
    CONFIG.map_size = map_size;

    debug("Parsing list file...\n");
    /* Allocate pointer array now when we know its size */
    dw_list->map = calloc_assert(CONFIG.map_size, sizeof(struct dw_node*));
    for (int i = 0; i < map_size; ++i){
#if DEBUG
        if (i%(map_size/10) == 0){
            debug("Progress: %d%%\n",i/(map_size/100));
        }
#endif
        /* Key is followed by space */
        str = strtok(NULL, " ");
        if (str == NULL){
            error("Not enough entries present?\nIteration %d of %d\n", i, map_size);
            free(chunk);
            free(charset);
            return false;
        }
        if (strlen(str) != key_size){
            error("Length of key %s not consistent with given length of file", str);
        }
        char key[key_size];
        strcpy(key, str);
        /* Word is followed by newline (Theoretically will support phrases
         * in list I guess , but shouldn't happen if dw has generated the list) */
        str = strtok(NULL, "\n");
        if (str == NULL){
            error("Word missing from entry.\nIteration %d of %d\n", i, map_size);
            return false;
        }

        char *word = str_malloc(strlen(str));
        strcpy(word, str);

        /* Does key contain only characters in character set? */
        size_t p = strcspn(key, CONFIG.char_set);
        if (p != 0){
            error("Key %s is not valid in charset %s\n", key, CONFIG.char_set);
            free(chunk);
            free(charset);
            free(word);
            return false;
        }

        /* Create node and insert */
        struct dw_node *new = malloc_assert(sizeof(struct dw_node));
        new->value.id = str_malloc(key_size);
        new->value.value = str_malloc(strlen(word));
        strcpy(new->value.value, word);
        strcpy(new->value.id, key);
        new->key = str_hash(key, map_size);
        new->next = NULL;
        node_insert(&(dw_list->map[new->key]), new);
        free(word);
    }
    debug("Parsing complete.\n");
    free(charset);
    free(chunk);
    return true;
}
bool arguments_insert(struct arguments *arguments, int type, char *arg){
    /* Create pair */
    struct arg_pair *new = malloc_assert(sizeof(struct arg_pair));
    new->type = type;
    new->value = str_malloc(strlen(arg));
    strcpy(new->value, arg);
    new->next = NULL;

    /* Insert */
    struct arg_pair *node = arguments->arguments;
    if (node != NULL){
        while (node->next != NULL){
            node = node->next;
        }
        node->next = new;
    } else {
        arguments->arguments = new;
    }
    return true;
}
void args_free(struct arg_pair *node){
    if (node != NULL){
        args_free(node->next);
        free(node->value);
        free(node);
    }
}
char **parse_key_word(char *format, char *string){
    enum kwmode{NONE = 0, KEY, WORD} mode = NONE;
    /* Key-word pair */
    char **key_word = malloc_assert(sizeof(char*) * 2);
    int mode_start = 0;

    /* i for moving across format string, j to move across input string */
    for (int i = 0, j = 0; i < strlen(format); ++i){
        /* Switch between parsing key and word */
        if (format[i] == 'k'){
            mode = KEY;
        } else if (format[i] == 'w'){
            mode = WORD;
        } else {
            /* If separator, the start of the next key or word will be the next character */
            mode_start = ++j;
            mode = NONE;
        }
        switch (mode){
        case KEY:
            /* Find the end of the key,
             * format[i+1] should contain the delimiter at this point if not passed already */
            for (; (string[j] != format[i+1] && j <= strlen(string)); ++j);
            key_word[0] = str_malloc(j - mode_start);
            /* Copy (j - mode_start) characters starting from mode_start */
            strncpy(key_word[0], &string[mode_start], j - mode_start);
            break;
        case WORD:
            /* Find the end of the word,
             * format[i+1] should contain the delimiter at this point if not passed already */
            for (; (string[j] != format[i+1] && j <= strlen(string)); ++j);
            key_word[1] = str_malloc(j - mode_start);
            /* Copy (j - mode_start) characters starting from mode_start */
            strncpy(key_word[1], &string[mode_start], j - mode_start);
            break;
        case NONE:
        default:
            break;
        }
    }
    return key_word;
}
