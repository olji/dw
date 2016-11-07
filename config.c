#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <confuse.h>

#include "config.h"

#define CONFIGFILE "dw.conf"
#define DEF_LIST "default"
#define DEF_CHARSET "0123456789"

extern struct dw_config CONFIG;

int read_config(char *homedir){
    /* Set default values */
    CONFIG.default_list = malloc(sizeof(char)*strlen(DEF_LIST) + 1);
    strcpy(CONFIG.default_list, DEF_LIST);
    CONFIG.key_length = 5;
    CONFIG.char_set = malloc(sizeof(char)*strlen(DEF_CHARSET) + 1);
    strcpy(CONFIG.char_set, DEF_CHARSET);
    CONFIG.unique = true;
    CONFIG.word_min_len = 2;

    char *configpath = calloc(strlen(homedir) + strlen(CONFIGFILE) + 1, sizeof(char));
    strcpy(configpath, homedir);
    strcat(configpath, CONFIGFILE);
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
    fprintf(cf, "# The list used if omitted on command line\ndefault-list = \"%s\"\n# Character set used for word keys of diceware lists created from using -c\ncharacter-set = \"%s\"\n# Length of key for every word during creation of diceware lists by using -c\nkey-length = %ld\n# If true, enforces the need of unique words in the diceware list, \n# HIGHLY RECOMMENDED to leave true as duplicate words weaken the security of generated passphrases\nunique-words = %s\n# Minimum word length of each word in the diceware list, \n# it is recommended to have a length of at least 14 characters, \n# including spaces, for a passphrase of 5 words and 17 characters for \n# 6 words, making a minimum word length of 2 ensure every passphrase \n# should follow that length, if set to 1 it is recommended to manually \n# ensure that the passphrase keeps a good length (Average of two characters per word or higher).\nmin-word-length = %ld",CONFIG.default_list, CONFIG.char_set, CONFIG.key_length, (CONFIG.unique ? "true": "false"), CONFIG.word_min_len);
    fclose(cf);
    return true;
}
void free_conf(){
    free(CONFIG.default_list);
    free(CONFIG.char_set);
}
