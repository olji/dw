#ifndef CONFIG_H
#define CONFIG_H

#include <confuse.h>

struct dw_config{
    char *default_list;
    long int key_length;
    char *char_set;
    size_t char_set_size;
    size_t map_size;
    cfg_bool_t unique;
    long int word_min_len;
} CONFIG;

int read_config(char*);
bool write_default_config(char*);
void free_conf();

#endif
