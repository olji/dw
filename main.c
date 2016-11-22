#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "map.h"
#include "config.h"

#define DEF_HOME "/.dw/"

extern struct dw_config CONFIG;

const char *argp_program_version = "dw 0.4";
const char *argp_program_bug_address = "<N/A>";

static char doc[] = "dw - Diceware manager";

static char args_doc[] = "[LIST]";

struct arg_pair {
    enum {NONE = 0, INPUT_FILE, PP_LENGTH} type;
    char *value;
    struct arg_pair *next;
};
struct arguments {
    enum {LIST_NONE = 0, CREATE, IMPORT} list_option;
    enum {DW_NONE = 0, GEN, LOOK} dw_option;
    bool use_list;
    char *list;
    struct arg_pair *arguments;
    bool parse_failure;
};

void generate(struct dw_hashmap*, int);
void lookup(struct dw_hashmap*);
bool arguments_insert(struct arguments*, int, char*);
int list_create(FILE*, FILE*, struct dw_hashmap*);
void list_import(FILE*, FILE*, struct dw_hashmap*);
bool list_parse(FILE*, struct dw_hashmap*);
void args_free(struct arg_pair*);

static error_t parse_opt (int key, char *arg, struct argp_state *state){
    struct arguments *arguments = state->input;
    switch (key){
    case 'g':
        arguments->dw_option = GEN;
        if (arg != NULL){
            for (int i = 0; i < strlen(arg); ++i){
                if (arg[i] < '0' || arg[i] > '9'){
                    printf("ERR: Optional parameter to -g (--generate) should only be integers\n");
                    arguments->parse_failure = true;
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
#if DEBUG
        printf("Input file: %s\n", arg);
#endif
        arguments_insert(arguments, INPUT_FILE, arg);
        break;
    case 'i':
        arguments->list_option = IMPORT;
        printf("Import is not implemented yet, exiting...\n");
        exit(0);
        break;
    case 'u':
        arguments->list = malloc(sizeof(char)*(strlen(arg) + 1));
        strcpy(arguments->list, arg);
        arguments->use_list = true;
        break;
    case ARGP_KEY_ARG:
        /* TODO: Keep inside span of dw home directory, deny traversion upwards using .. */
        arguments->list = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp_option options[] = {
    { "generate", 'g', "PASSPHRASE_WORD_COUNT", OPTION_ARG_OPTIONAL, "Generate a passphrase using LIST", 1 },
    { "lookup", 'l', "DW_NUM", OPTION_ARG_OPTIONAL, "Look up passphrase using LIST", 1 },
    { "import-list", 'i', "FILE", 0, "Import diceware list named LIST", 0 },
    { "create-list", 'c', "FILE", 0, "Create a new diceware list named LIST", 0 },
    { "use-list", 'u', "LIST", 0, "Use given LIST not present in home directory of dw", 0 },
    { 0 }
};

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv){
#if DEBUG == 1
    printf("DEBUG IS ON\n===================\n");
#endif
    if (argc < 2){
        printf("No function asked for.\n\n");
        argp_help(&argp, stdout, ARGP_HELP_SHORT_USAGE, "dw");
        argp_help(&argp, stdout, ARGP_HELP_LONG, "dw");
        return 0;
    }

    struct arguments input_args = {LIST_NONE, DW_NONE, false, NULL, NULL, 0, false};
    argp_parse (&argp, argc, argv, 0, 0, &input_args);
    if (input_args.parse_failure){
        return 1;
    }
    char *home = getenv("DW_HOME");
    char *listpath;
    if (home == NULL){
        printf("NOTE: DW_HOME is not set, assuming ~/.dw/\n");
        char *usr_home = getenv("HOME");
        listpath = calloc((strlen(usr_home) + strlen(DEF_HOME) + 1), sizeof(char));
        strcat(listpath, usr_home);
        strcat(listpath, DEF_HOME);
    } else {
        listpath = malloc(sizeof(char) * (strlen(home) + 1));
        strcat(listpath, home);
    }
    int exit_status = read_config(listpath);
    if (exit_status < 1){
        return abs(exit_status);
    }

    if (input_args.list == NULL){
        printf("Using default list...\n");
        if (CONFIG.default_list != NULL){
            input_args.list = malloc(sizeof(char) * (strlen(CONFIG.default_list) + 1));
            strcpy(input_args.list, CONFIG.default_list);
        } else {
            printf("ERROR: default list not set, set default list in configuration or provide list name manually\n");
            return 1;
        }
    }

#if DEBUG
    printf("List file used: %s\n", input_args.list);
#endif
    if (input_args.use_list){
        free(listpath);
        listpath = calloc(strlen(input_args.list) + 1, sizeof(char));
        strcat(listpath, input_args.list);
    } else {
        char *tmp = calloc(strlen(listpath) + strlen(input_args.list) + 1, sizeof(char));
        strcat(tmp, listpath);
        strcat(tmp, input_args.list);
        free(listpath);
        listpath = tmp;
    }
#if DEBUG
    printf("Listpath: %s\n", listpath);
#endif

    char *input_file;
    int g_length = 0;

    struct arg_pair *node = input_args.arguments;
    while (node != NULL){
        if (node->type == INPUT_FILE){
            input_file = node->value;
        } else if (node->type == PP_LENGTH){
            char* end;
            g_length = (int)strtol(node->value, &end, 10);
        }
        node = node->next;
    }
    FILE *list;

    /*
     * We currently cannot be sure of the actual size of the pointer array,
     * so we wait with allocating memory for that.
     */
    struct dw_hashmap *dw_list = malloc(sizeof(struct dw_hashmap));
    if (input_args.list_option != LIST_NONE){
        list = fopen(listpath, "r");
        if (list != NULL){
            fclose(list);
            /* TODO: Refuse to delete file if supplied using -u */
            printf("File already exists: %s, delete? [y/n]: ", listpath);
            char ans = 0;
            do{
                if (ans != 0){
                    printf("Please answer y or n\n");
                }
                /*
                 * Scanf sets the value of input_args.list_option to zero for some reason here,
                 * workaround until actual issue found and corrected
                 */
                int tmp = input_args.list_option;
                scanf("%s", &ans);
                input_args.list_option = tmp;
                ans = tolower(ans);
            } while (ans != 'y' && ans != 'n');
            if (ans == 'y'){
                remove(listpath);
            } else {
                return 1;
            }
        }
        FILE *input = fopen(input_file, "r");
        if (input == NULL){
            printf("ERROR: Could not open input file %s\n", input_file);
            return 1;
        }
        switch (input_args.list_option){
        case CREATE:;
            exit_status = list_create(list, input, dw_list);
            if (exit_status < 1){
                return abs(exit_status);
            }
            break;
        case IMPORT:
            list_import(list, input, dw_list);
            break;
        default:
            printf("list_option default case reached, exiting.\n");
            return 2;
        }
        fclose(input);
        list = fopen(listpath, "wx");
        map_write(list, dw_list);
        fclose(list);
    }

    if (input_args.dw_option != DW_NONE){
        if (input_args.list_option == LIST_NONE){
            list = fopen(listpath, "r");
            if (list == NULL){
                printf("ERROR: Failed to open list %s for reading.\n", listpath);
                return 1;
            }
#if DEBUG
            else {
                printf("Opened: %s\n", listpath);
            }
#endif
            bool ret = list_parse(list, dw_list);
            fclose(list);
            if (!ret){
                printf("Exiting...\n");
                return 1;
            }
        } else {
            map_rearrange(dw_list);
        }

        switch (input_args.dw_option){
        case GEN:
            generate(dw_list, g_length);
            break;
        case LOOK:
            lookup(dw_list);
            break;
        default:
            printf("dw_option default case reached, exiting.");
            return 2;
        }
    }

    /* Cleanup */
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
    char *id = calloc(length + 1, sizeof(char));
    srand(time(NULL));
    char *passphrase = calloc(1, sizeof(char));
    char *pw_id = calloc((length*CONFIG.key_length) + length + 1, sizeof(char));

    for (int i = 0; i < length; ++i){
        for (int j = 0; j < CONFIG.key_length; ++j){
            id[j] = CONFIG.char_set[rand()%CONFIG.char_set_size];
        }
        strcat(pw_id, id);
        strcat(pw_id, " ");

        char *found_word = map_lookup(dw_list, id);
        char *word_copy = calloc(strlen(found_word) + 1, sizeof(char));
        strcpy(word_copy, found_word);
        /* Using sizeof instead of strlen can be used to avoid addition by two, but is not as readable */
        char *tmp_pw = calloc(strlen(passphrase) + strlen(word_copy) + 2, sizeof(char));
        strcat(tmp_pw, passphrase);
        strcat(tmp_pw, " ");
        strcat(tmp_pw, word_copy);
        free(passphrase);
        free(word_copy);
        passphrase = tmp_pw;
    }
    printf("Passphrase: %s\n", passphrase);
    printf("Passphrase ID: %s\n", pw_id);
    free(id);
    free(pw_id);
    free(passphrase);
}
void lookup(struct dw_hashmap *dw_list){
    char pw[50];
    while (scanf("%s", pw) != EOF){
        char *ret = map_lookup(dw_list, pw);
        printf("%s\n", ret);
    }
}
int list_create(FILE *map, FILE *input_file, struct dw_hashmap *dw_list){
    if (!CONFIG.unique){
        printf("NOTE: dw is currently set to NOT enforce unique words in the diceware list, which may result in duplicate entries, really continue? [y/n]: ");
        char ans = 0;
        do{
            if (ans != 0){
                printf("Please answer y or n\n");
            }
            scanf("%s", &ans);
            ans = tolower(ans);
        } while (ans != 'y' && ans != 'n');
        if (ans == 'n'){
            return 0;
        }
    }
    fseek(input_file, 0, SEEK_END);
    int f_size = ftell(input_file);
    rewind(input_file);
    dw_list->map = calloc(CONFIG.map_size, sizeof(struct dw_node*));

    char *delims = " \n";
    char *chunk = malloc(f_size);
    fread(chunk, f_size, 1, input_file);
    char *str = strtok(chunk, delims);
    while (str != NULL){
        if (strlen(str) >= CONFIG.word_min_len){
            map_insert(dw_list, str);
        }
        if (map_filled()){
            break;
        }
        str = strtok(NULL, delims);
    }
    if (!map_filled()){
        printf("ERR: Could not gather %zu %swords from input file, lower key size, present a file with more %swords or reduce the size of your character set.%s\n", CONFIG.map_size, (CONFIG.unique?"unique ":""), (CONFIG.unique?"unique ":""), (CONFIG.unique?"\nEnabling duplicate words by setting unique_words to false in may result in gathering enough words, but WILL weaken the security of passphrases generated":""));
        return -1;
    }
    free(chunk);
    return 1;
}
void list_import(FILE *list, FILE *input_file, struct dw_hashmap *dw_list){
}
bool list_parse(FILE *list, struct dw_hashmap *dw_list){
    fseek(list, 0, SEEK_END);
    int f_size = ftell(list);
    rewind(list);

    char *chunk = malloc(f_size);
    fread(chunk, f_size, 1, list);
    char *str = strtok(chunk, "\n");

    size_t key_size = 0, charset_size = 0;
#if DEBUG
    printf("chunk: %s, f_size: %d, chunk_part: %s\n", chunk, f_size, str);
#endif

    sscanf(str, "%zu-%zu", &key_size, &charset_size);

    char *charset = malloc(sizeof(char) * (charset_size + 1));
    const int map_size = pow(charset_size, key_size);

    str = strtok(NULL, "\n");
    sscanf(str, "%s", charset);

    free(CONFIG.char_set);

    CONFIG.key_length = key_size;
    CONFIG.char_set = malloc(sizeof(char) * (strlen(charset) + 1));
    strcpy(CONFIG.char_set, charset);
    CONFIG.char_set_size = charset_size;
    CONFIG.map_size = map_size;

#if DEBUG
    printf("Parsing list file...\n");
#endif
    /* Allocate pointer array now when we know its size */
    dw_list->map = calloc(CONFIG.map_size, sizeof(struct dw_node*));
    for (int i = 0; i < map_size; ++i){
#if DEBUG
        if (i%(map_size/10) == 0){
            printf("Progress: %d%%\n",i/(map_size/100));
        }
#endif
        str = strtok(NULL, "\n");
        if (str == NULL){
            printf("Not enough entries present?\nIteration %d of %d\n", i, map_size);
            return false;
        }
        char key[key_size];
        sscanf(str, "%s", key);
        str = strtok(NULL, "\n");
        if (str == NULL){
            printf("Entry not complete? (Word missing)\n");
            return false;
        }

        struct dw_node *new = malloc(sizeof(struct dw_node));
        new->value.id = malloc(sizeof(char) * (key_size + 1));
        new->value.value = malloc(sizeof(char) * (strlen(str) + 1));
        strcpy(new->value.value, str);
        strcpy(new->value.id, key);
        new->key = str_hash(key, map_size);
        new->next = NULL;
#if DEBUG
        printf("map size: %zu\n", CONFIG.map_size);
        printf("key hash: %zu\n", new->key);
#endif
        node_insert(&(dw_list->map[new->key]), new);
    }
#if DEBUG
    printf("Parsing complete.\n");
#endif
    free(charset);
    free(chunk);
    return true;
}
bool arguments_insert(struct arguments *arguments, int type, char *arg){
    struct arg_pair *new = malloc(sizeof(struct arg_pair));
    new->type = type;
    new->value = calloc(strlen(arg) + 1, sizeof(char));
    strcpy(new->value, arg);
    new->next = NULL;

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
