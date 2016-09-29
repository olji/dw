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

const char *argp_program_version = "dw 0.0";
const char *argp_program_bug_address = "<N/A>";

static char doc[] = "dw - Diceware manager";

static char args_doc[] = "[TABLE]";

struct arguments {
    enum {GEN = 0, LOOK, CREATE, IMPORT} mode;
    char *table;
    char *argument;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state){
    struct arguments *arguments = state->input;
    switch (key){
    case 'g':
        arguments->mode = GEN;
        break;
    case 'l':
        arguments->mode = LOOK;
        break;
    case 'c':
        arguments->mode = CREATE;
        break;
    case 'i':
        arguments->mode = IMPORT;
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

    struct arguments input_args;
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
        if (input_args.table == NULL){
            if (table_default != NULL){
                input_args.table = table_default;
                /* Check if file exists in $DW_HOME/tables/ */
            } else {
                printf("ERROR: DW_DEF_TABLE not set, set environment variable or provide table name manually\n");
                return 1;
            }
        }
    }
    FILE *table = fopen(strcat(home, input_args.table), "r");
    FILE *input;
    if (input_args.mode >= CREATE){
        if (table != NULL){
            printf("Table with that name already exists");
            return 1;
        }
        table = fopen(strcat(home, input_args.table), "wx");
        input = fopen(input_args.argument, "r");
        if (input == NULL){
            printf("ERROR: Could not open input file %s\n", input_args.argument);
            return 1;
        }
    }
    if (table == NULL){
        printf("ERROR: Could not open file %s\n", home);
        return 1;
    }
    switch (input_args.mode){
    case GEN:
        generate(table);
        break;
    case LOOK:
        lookup(table, input_args.argument);
        break;
    case CREATE:;
        create_table(table, input);
        break;
    case IMPORT:;
        import_table(table, input);
    default:
        printf("wut");
        return 2;
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
