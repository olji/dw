#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "table.h"

void generate(struct dw_hashmap*);
void lookup(struct dw_hashmap*, char*);
void create_table(FILE*, FILE*, struct dw_hashmap*);
void import_table(FILE*, FILE*, struct dw_hashmap*);
bool parse_table(FILE*, struct dw_hashmap*);
void table_write(FILE*, struct dw_hashmap*);
/* Helper function to write table */
void node_write(FILE*, struct dw_node*);
void node_print(struct dw_node *node);
void print_table(struct dw_hashmap *table);

const char *argp_program_version = "dw 0.1";
const char *argp_program_bug_address = "<N/A>";

static char doc[] = "dw - Diceware manager";

static char args_doc[] = "[TABLE]";

struct arguments {
    enum {TABLE_NONE = 0, CREATE, IMPORT} table_option;
    enum {DW_NONE = 0, GEN, LOOK} dw_option;
    char *table;
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
        arguments->table_option = CREATE;
        break;
    case 'i':
        arguments->table_option = IMPORT;
        break;
    case ARGP_KEY_ARG:
        arguments->table = arg;
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
    { "generate", 'g', 0, 0, "Generate a passphrase using TABLE", 1 },
    { "lookup", 'l', "DW_NUM", OPTION_ARG_OPTIONAL, "Look up passphrase using TABLE", 1 },
    { "import-table", 'i', "FILE", 0, "Import diceware table named TABLE", 0 },
    { "create-table", 'c', "FILE", 0, "Create a new diceware table named TABLE", 0 },
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

    struct arguments input_args = {TABLE_NONE, DW_NONE, NULL, NULL};
    argp_parse (&argp, argc, argv, 0, 0, &input_args);
    char *home = getenv("DW_HOME");
    if (home == NULL){
        printf("NOTE: DW_HOME is not set, using ~/.dw/\n");
        char *usr_home = getenv("HOME");
        home = strcat(usr_home, "/.dw/");
    }

    if (input_args.table == NULL){
        printf("Using default table...\n");
        char *table_default = getenv("DW_DEF_TABLE");
        if (table_default != NULL){
            input_args.table = table_default;
            /* Check if file exists in $DW_HOME/tables/ */
        } else {
            printf("ERROR: DW_DEF_TABLE not set, set environment variable or provide table name manually\n");
            return 1;
        }
    }

    strcat(home, input_args.table);

    FILE *table;
    struct dw_hashmap *dw_list = malloc(sizeof(struct dw_hashmap));
    dw_list->map = malloc(sizeof(struct dw_node*)*TABLE_SIZE);
    if (input_args.table_option != TABLE_NONE){
        table = fopen(home, "r");
        if (table != NULL){
            printf("File already exists: %s\n", home);
            fclose(table);
            return 1;
        }
        FILE *input = fopen(input_args.argument, "r");
        if (input == NULL){
            printf("ERROR: Could not open input file %s\n", input_args.argument);
            return 1;
        }
        switch (input_args.table_option){
        case CREATE:
            create_table(table, input, dw_list);
            break;
        case IMPORT:
            printf("Importing\n");
            import_table(table, input, dw_list);
            break;
        default:
            printf("table_option default case reached, exiting.");
            return 2;
        }
        fclose(input);
        table = fopen(home, "wx");
        table_write(table, dw_list);
        fclose(table);
    }

    if (input_args.dw_option != DW_NONE){
        if (input_args.table_option == TABLE_NONE){
            table = fopen(home, "r");
            if (table == NULL){
                printf("ERROR: Failed to open file %s for reading.\n", home);
                return 1;
            }
            bool ret = parse_table(table, dw_list);
            fclose(table);
            if (!ret){
                printf("Exiting...\n");
                return 1;
            }
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
    dw_delete_map(dw_list);
    return 0;
}

void generate(struct dw_hashmap *dw_list){
    size_t length = 0;

    printf("How many words? ");
    scanf("%zu", &length);
    char *id = calloc(length, sizeof(char) + 1);
    srand(time(NULL));
    char *passphrase = calloc(1, sizeof(char));
    char *pw_id = calloc((length*KEY_LENGTH) + length + 1, sizeof(char));

    for (int i = 0; i < length; ++i){
        for (int j = 0; j < KEY_LENGTH; ++j){
            id[j] = CHAR_SET[rand()%strlen(CHAR_SET)];
        }
        strcat(pw_id, id);
        strcat(pw_id, " ");

        char *found_word = dw_map_lookup(dw_list, id);
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
    free(pw_id);
    free(passphrase);
}
void lookup(struct dw_hashmap *dw_list, char *pw_num){
}
void create_table(FILE *table, FILE *input_file, struct dw_hashmap *dw_list){
    fseek(input_file, 0, SEEK_END);
    int f_size = ftell(input_file);
    rewind(input_file);

    char *delims = " \n";
    char *chunk = malloc(f_size);
    fread(chunk, f_size, 1, input_file);
    char *str = strtok(chunk, delims);
    while (str != NULL){
        dw_map_insert(dw_list, str);
        if (map_filled()){
            break;
        }
        str = strtok(NULL, delims);
    }
    free(chunk);
}
void import_table(FILE *table, FILE *input_file, struct dw_hashmap *dw_list){
}
bool parse_table(FILE *table, struct dw_hashmap *dw_list){
    fseek(table, 0, SEEK_END);
    int f_size = ftell(table);
    rewind(table);

    char *chunk = malloc(f_size);
    fread(chunk, f_size, 1, table);
    char *str = strtok(chunk, "\n");

    size_t key_size, charset_size;

    sscanf(str, "%zu-%zu", &key_size, &charset_size);

    printf("key_size: %zu, charset_size: %zu\n", key_size, charset_size);
    char *charset = malloc(sizeof(char)*charset_size+1);
    const int table_size = pow(charset_size, key_size);

    str = strtok(NULL, "\n");
    sscanf(str, "%s", charset);
    printf("charset: '%s'\n", charset);

    for (int i = 0; i < table_size; ++i){
        str = strtok(NULL, "\n");
        if (str == NULL){
            printf("Not enough entries present?\nIteration %d of %d", i, table_size);
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
        new->value.value = malloc(sizeof(char)*strlen(word)+1);
        strcpy(new->value.value, word);
        strcpy(new->value.id, key);
        new->key = str_hash(key, table_size);
        new->next = NULL;
        dw_node_insert(&(dw_list->map[new->key]), new);
        free(word);
    }
    free(charset);
    free(chunk);
    return true;
}
void node_print(struct dw_node *node){
    if (node == NULL){
        printf("[NULL]\n");
        return;
    }
    printf("[%d:[%s]]->", node->key, node->value.id);
    node_print(node->next);
}
void print_table(struct dw_hashmap *table){
    for (int i = 0; i < TABLE_SIZE; ++i){
        printf("[%d]->", i);
        node_print(table->map[i]);
    }
}
/* 
 * Mainly used for debugging purposes, may prove useful 
 * for other things however 
 */
void node_write(FILE *fp, struct dw_node *node){
    if (node == NULL){
        return;
    }
    fprintf(fp, "%s\n%zu\n%s\n",node->value.id, strlen(node->value.value), node->value.value);
    node_write(fp, node->next);
}
void table_write(FILE *fp, struct dw_hashmap *table){
    fprintf(fp, "%zu-%zu\n", (size_t)KEY_LENGTH, strlen(CHAR_SET));
    fprintf(fp, "%s\n", CHAR_SET);
    for (int i = 0; i < TABLE_SIZE; ++i){
        node_write(fp, table->map[i]);
    }
}
