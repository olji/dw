#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>

#include "table.h"

void generate(FILE*);
void lookup(FILE*, char*);
void create_table(FILE*, FILE*);
void import_table(FILE*, FILE*);
struct table *parse_table(FILE*);
int write_table(FILE*, struct table*);

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
        struct dw_hashmap *dw_list = malloc(sizeof(struct dw_hashmap));
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
         *fclose(table);
         */
    }

    if (input_args.dw_option != DW_NONE){
        table = fopen(home, "r");

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
        fclose(table);
    }
    return 0;
}

void generate(FILE *table){
}
void lookup(FILE *table, char *pw_num){
}
void create_table(FILE *table, FILE *input_file){
}
void import_table(FILE *table, FILE *input_file){
}
struct table *parse_table(FILE *table){
    return NULL;
}
int write_table(FILE* fp, struct table* table){
    return -1;
}
