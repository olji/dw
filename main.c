#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "map.h"
#include "config.h"

extern struct dw_config CONFIG;

void generate(struct dw_hashmap*);
void lookup(struct dw_hashmap*, char*);
int  list_create(FILE*, FILE*, struct dw_hashmap*);
void list_import(FILE*, FILE*, struct dw_hashmap*);
bool list_parse(FILE*, struct dw_hashmap*);

const char *argp_program_version = "dw 0.2";
const char *argp_program_bug_address = "<N/A>";

static char doc[] = "dw - Diceware manager";

static char args_doc[] = "[LIST]";

struct arguments {
    enum {LIST_NONE = 0, CREATE, IMPORT} list_option;
    enum {DW_NONE = 0, GEN, LOOK} dw_option;
    char *list;
    char *argument;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state){
    struct arguments *arguments = state->input;
    switch (key){
    case 'g':
        arguments->dw_option = GEN;
        break;
    case 'l':
        arguments->dw_option = LOOK;
        break;
    case 'c':
        arguments->list_option = CREATE;
        break;
    case 'i':
        arguments->list_option = IMPORT;
        break;
    case ARGP_KEY_ARG:
        arguments->list = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    if (key != ARGP_KEY_ARG && arg != NULL){
        arguments->argument = arg;
    }
    return 0;
}

static struct argp_option options[] = {
    /* TODO: Actually implement the optional word count */
    { "generate", 'g', "PASSPHRASE_WORD_COUNT", OPTION_ARG_OPTIONAL, "Generate a passphrase using LIST", 1 },
    { "lookup", 'l', "DW_NUM", OPTION_ARG_OPTIONAL, "Look up passphrase using LIST", 1 },
    { "import-list", 'i', "FILE", 0, "Import diceware list named LIST", 0 },
    { "create-list", 'c', "FILE", 0, "Create a new diceware list named LIST", 0 },
    { 0 }
};

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv){
    if (argc < 2){
        printf("No function asked for.\n\n");
        argp_help(&argp, stdout, ARGP_HELP_SHORT_USAGE, "dw");
        argp_help(&argp, stdout, ARGP_HELP_LONG, "dw");
        return 0;
    }

    struct arguments input_args = {LIST_NONE, DW_NONE, NULL, NULL};
    argp_parse (&argp, argc, argv, 0, 0, &input_args);
    char *home = getenv("DW_HOME");
    if (home == NULL){
        printf("NOTE: DW_HOME is not set, assuming ~/.dw/\n");
        char *usr_home = getenv("HOME");
        home = strcat(usr_home, "/.dw/");
    }
    int exit_status = read_config(home);
    if (exit_status < 1){
        return abs(exit_status);
    }

    if (input_args.list == NULL){
        printf("Using default list...\n");
        if (CONFIG.default_list != NULL){
            input_args.list = malloc(sizeof(char)*(strlen(CONFIG.default_list)+1));
            strcpy(input_args.list, CONFIG.default_list);
        } else {
            printf("ERROR: default list not set, set default list in configuration or provide list name manually\n");
            return 1;
        }
    }

    strcat(home, input_args.list);

    FILE *list;
    struct dw_hashmap *dw_list = malloc(sizeof(struct dw_hashmap));
    dw_list->map = calloc(CONFIG.map_size, sizeof(struct dw_node*));
    if (input_args.list_option != LIST_NONE){
        list = fopen(home, "r");
        if (list != NULL){
            fclose(list);
            printf("File already exists: %s, delete? [y/n]: ", home);
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
                remove(home);
            } else {
                return 1;
            }
            printf("List option: %d\n", input_args.list_option);
        }
        FILE *input = fopen(input_args.argument, "r");
        if (input == NULL){
            printf("ERROR: Could not open input file %s\n", input_args.argument);
            return 1;
        }
        switch (input_args.list_option){
        case CREATE:;
            int retval = list_create(list, input, dw_list);
            if (retval < 1){
                return abs(retval);
            }
            break;
        case IMPORT:
            printf("Importing\n");
            list_import(list, input, dw_list);
            break;
        default:
            printf("list_option default case reached, exiting.\n");
            return 2;
        }
        fclose(input);
        list = fopen(home, "wx");
        map_write(list, dw_list);
        fclose(list);
    }

    if (input_args.dw_option != DW_NONE){
        if (input_args.list_option == LIST_NONE){
            list = fopen(home, "r");
            if (list == NULL){
                printf("ERROR: Failed to open file %s for reading.\n", home);
                return 1;
            }
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
            generate(dw_list);
            break;
        case LOOK:
            lookup(dw_list, input_args.argument);
            break;
        default:
            printf("dw_option default case reached, exiting.");
            return 2;
        }
    }
    /*
     *print_table(dw_list);
     */
    map_delete(dw_list);
    free_conf();
    return 0;
}

void generate(struct dw_hashmap *dw_list){
    size_t length = 0;

    printf("How many words? ");
    scanf("%zu", &length);
    char *id = calloc(length, sizeof(char) + 1);
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
void lookup(struct dw_hashmap *dw_list, char *pw_num){
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

    size_t key_size, charset_size;

    sscanf(str, "%zu-%zu", &key_size, &charset_size);

    char *charset = malloc(sizeof(char)*charset_size+1);
    const int map_size = pow(charset_size, key_size);

    str = strtok(NULL, "\n");
    sscanf(str, "%s", charset);

    free(CONFIG.char_set);

    CONFIG.key_length = key_size;
    CONFIG.char_set = malloc(sizeof(char) * (strlen(charset) + 1));
    strcpy(CONFIG.char_set, charset);
    CONFIG.char_set_size = charset_size;
    CONFIG.map_size = map_size;

    for (int i = 0; i < map_size; ++i){
        str = strtok(NULL, "\n");
        if (str == NULL){
            printf("Not enough entries present?\nIteration %d of %d", i, map_size);
            return false;
        }
        char key[key_size];
        size_t word_size;
        sscanf(str, "%s", key);
        str = strtok(NULL, "\n");
        sscanf(str, "%zu", &word_size);

        str = strtok(NULL, "\n");
        if (str == NULL){
            printf("Entry not complete?\n");
            return false;
        }
        char *word = malloc(sizeof(char)*word_size+1);
        sscanf(str, "%s", word);
        struct dw_node *new = malloc(sizeof(struct dw_node));
        new->value.id = malloc(sizeof(char)*key_size+1);
        new->value.value = malloc(sizeof(char)*strlen(word)+1);
        strcpy(new->value.value, word);
        strcpy(new->value.id, key);
        new->key = str_hash(key, map_size);
        new->next = NULL;
        node_insert(&(dw_list->map[new->key]), new);
        free(word);
    }
    free(charset);
    free(chunk);
    return true;
}
