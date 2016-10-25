#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>

#include "table.h"

void generate(FILE*);
void lookup(FILE*, char*);
void create_table(FILE*, FILE*, struct dw_hashmap*);
void import_table(FILE*, FILE*, struct dw_hashmap*);
void parse_table(FILE*, struct dw_hashmap*);
void table_write(FILE*, struct dw_hashmap*);
/* Helper function to write table */
void node_write(FILE*, struct dw_node*);
void node_print(struct dw_node *node);
void print_table(struct dw_hashmap *table);

const char *argp_program_version = "dw 0.0";
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
        /*
         *table = fopen(home, "wx");
         *table_write(table, dw_list);
         *fclose(table);
         */
    }

    if (input_args.dw_option != DW_NONE){
        if (input_args.table_option == TABLE_NONE){
            table = fopen(home, "r");
            parse_table(table, dw_list);
            fclose(table);
        }

        switch (input_args.dw_option){
        case GEN:
            generate(table);
            break;
        case LOOK:
            lookup(table, input_args.argument);
            break;
        default:
            printf("dw_option default case reached, exiting.");
            return 2;
        }
    }
    dw_delete_map(dw_list);
    return 0;
}

void generate(FILE *table){
}
void lookup(FILE *table, char *pw_num){
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
void parse_table(FILE *table, struct dw_hashmap *dw_list){
    int key_size;
    char charset[40];
    fscanf(table, "%d-%s", &key_size, charset);
    printf("%d-%s\n", key_size, charset);
    /*
     *for (int i
     *    struct dw_node *new = malloc(sizeof(struct dw_node));
     *    new->key = pos;
     *    strcpy(new->value.id, given_key);
     *    new->value.value = word; 
     *}
     */
}
void node_print(struct dw_node *node){
    if (node == NULL){
        return;
    }
    printf("%s:%s\n",node->value.id, node->value.value);
    node_print(node->next);
}
void print_table(struct dw_hashmap *table){
    for (int i = 0; i < TABLE_SIZE; ++i){
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
    fprintf(fp, "%s:%s\n",node->value.id, node->value.value);
    node_write(fp, node->next);
}
void table_write(FILE *fp, struct dw_hashmap *table){
    fprintf(fp, "%d-%s\n", KEY_LENGTH, CHAR_SET);
    for (int i = 0; i < TABLE_SIZE; ++i){
        node_write(fp, table->map[i]);
    }
}
