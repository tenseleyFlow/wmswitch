#ifndef WMSWITCH_TOML_CONFIG_H
#define WMSWITCH_TOML_CONFIG_H

#include "../../lib/tomlc99/toml.h"

typedef struct {
    toml_table_t *root;
    char *filepath;
} wmswitch_config_t;

typedef struct {
    char *name;
    int number;
    char *monitor;
    char **rules;
    int rule_count;
} wmswitch_workspace_t;

typedef struct {
    char *key_combo;
    char *command;
    char *wm_exclusive;
} wmswitch_keybind_t;

typedef struct {
    wmswitch_workspace_t *workspaces;
    int workspace_count;
    wmswitch_keybind_t *keybinds;
    int keybind_count;
} wmswitch_parsed_config_t;

wmswitch_config_t* wmswitch_config_load(const char *filepath);
void wmswitch_config_free(wmswitch_config_t *config);

wmswitch_parsed_config_t* wmswitch_config_parse(wmswitch_config_t *config);
void wmswitch_parsed_config_free(wmswitch_parsed_config_t *parsed);

int wmswitch_config_validate(wmswitch_parsed_config_t *config);

#endif