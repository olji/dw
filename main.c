#include <stdio.h>
#include <stdlib.h>
#include <argp.h>

void generate();
void lookup();
void create_table();
char **parse_table();

const char *argp_program_version = "dw 0.0";
const char *argp_program_bug_address = "<N/A>";

static char doc[] = "dw - Diceware manager";

struct arguments {
    enum {GEN = 0, LOOK, CREATE, IMPORT} mode;
    char *args[2];
} arguments;

static char args_doc[] = "";

static struct argp_option options[] = {
    { "generate", 'g', "TABLE", 1, "Generate a passphrase using TABLE, uses $DW_DEF_TABLE if no table is provided" },
    { "lookup", 'l', "TABLE, ID", 1, "Look up passphrase using TABLE, uses $DW_DEF_TABLE" },
    { "import-table", 'i', "NAME, INPUTFILE", 0, "Import diceware table into $DW_HOME/tables/NAME" },
    { "create-table", 'c', "NAME, INPUTFILE", 0, "Create a new diceware table in $DW_HOME/tables/NAME" }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state){
    switch (key){
    case 'g':
        arguments.mode = GEN;
        break;
    case 'l':
        arguments.mode = LOOK;
        break;
    case 'c':
        arguments.mode = CREATE;
        break;
    case 'i':
        arguments.mode = IMPORT;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    if (arg != NULL){
        arguments.args[1] = arg;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char** argv){
    if (argc < 2){
        printf("No option given, nothing will be done.\n");
        argp_help(&argp, stdout, ARGP_HELP_LONG, "dw");
        return 0;
    }
    struct arguments input_args;
    argp_parse (&argp, argc, argv, 0, 0, &input_args);
    char* home = getenv("DW_HOME");
    char* table_default = getenv("DW_DEF_TABLE");
    if (home == NULL){
        printf("NOTE: DW_HOME is not set\n");
    }
    if (arguments.args[1] == NULL){
        if (table_default != NULL){
            arguments.args[1] = table_default;
            /* Check if file exists in $DW_HOME/tables/ */
        } else {
            printf("ERROR: DW_DEF_TABLE not set, set environment variable or provide table name manually\n");
            return 1;
        }
    }
    switch (arguments.mode){
    case GEN:
        generate();
        break;
    case LOOK:
        lookup();
        break;
    case CREATE:
        create_table();
        break;
    default:
        printf("wut");
        return 2;
    }
    return 0;
}

void generate(){
    printf("Generating shit yes...\n");
}
void lookup(){
    printf("Looking up shit yes...\n");
}
void create_table(){
    printf("Creating shit tables yes...\n");
}
