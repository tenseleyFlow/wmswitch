#include "wmswitch/toml_config.h"
#include "wmswitch/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

wmswitch_config_t* wmswitch_config_load(const char *filepath) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        wmswitch_log_error("Failed to open config file: %s", filepath);
        return NULL;
    }

    char errbuf[200];
    toml_table_t *root = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);
    
    if (!root) {
        wmswitch_log_error("Failed to parse TOML: %s", errbuf);
        return NULL;
    }

    wmswitch_config_t *config = malloc(sizeof(wmswitch_config_t));
    if (!config) {
        toml_free(root);
        return NULL;
    }

    config->root = root;
    config->filepath = strdup(filepath);
    
    wmswitch_log_info("Successfully loaded config from: %s", filepath);
    return config;
}

void wmswitch_config_free(wmswitch_config_t *config) {
    if (!config) return;
    
    if (config->root) {
        toml_free(config->root);
    }
    if (config->filepath) {
        free(config->filepath);
    }
    free(config);
}

wmswitch_parsed_config_t* wmswitch_config_parse(wmswitch_config_t *config) {
    if (!config || !config->root) {
        wmswitch_log_error("Invalid config provided to parser");
        return NULL;
    }

    wmswitch_parsed_config_t *parsed = calloc(1, sizeof(wmswitch_parsed_config_t));
    if (!parsed) {
        wmswitch_log_error("Failed to allocate memory for parsed config");
        return NULL;
    }

    toml_table_t *workspaces_table = toml_table_in(config->root, "workspaces");
    if (workspaces_table) {
        int workspace_count = 0;
        const char *key;
        
        for (int i = 0; (key = toml_key_in(workspaces_table, i)) != NULL; i++) {
            workspace_count++;
        }
        
        if (workspace_count > 0) {
            parsed->workspaces = calloc(workspace_count, sizeof(wmswitch_workspace_t));
            parsed->workspace_count = workspace_count;
            
            for (int i = 0; i < workspace_count; i++) {
                const char *ws_key = toml_key_in(workspaces_table, i);
                toml_table_t *ws_table = toml_table_in(workspaces_table, ws_key);
                
                parsed->workspaces[i].name = strdup(ws_key);
                
                toml_datum_t number = toml_int_in(ws_table, "number");
                if (number.ok) {
                    parsed->workspaces[i].number = number.u.i;
                }
                
                toml_datum_t monitor = toml_string_in(ws_table, "monitor");
                if (monitor.ok) {
                    parsed->workspaces[i].monitor = strdup(monitor.u.s);
                    free(monitor.u.s);
                }
            }
        }
    }

    toml_table_t *keybinds_table = toml_table_in(config->root, "keybinds");
    if (keybinds_table) {
        int keybind_count = 0;
        const char *key;
        
        for (int i = 0; (key = toml_key_in(keybinds_table, i)) != NULL; i++) {
            keybind_count++;
        }
        
        if (keybind_count > 0) {
            parsed->keybinds = calloc(keybind_count, sizeof(wmswitch_keybind_t));
            parsed->keybind_count = keybind_count;
            
            for (int i = 0; i < keybind_count; i++) {
                const char *kb_key = toml_key_in(keybinds_table, i);
                
                parsed->keybinds[i].key_combo = strdup(kb_key);
                
                toml_datum_t command = toml_string_in(keybinds_table, kb_key);
                if (command.ok) {
                    parsed->keybinds[i].command = strdup(command.u.s);
                    free(command.u.s);
                }
            }
        }
    }

    wmswitch_log_info("Parsed %d workspaces and %d keybinds", 
                     parsed->workspace_count, parsed->keybind_count);
    return parsed;
}

void wmswitch_parsed_config_free(wmswitch_parsed_config_t *parsed) {
    if (!parsed) return;
    
    for (int i = 0; i < parsed->workspace_count; i++) {
        free(parsed->workspaces[i].name);
        free(parsed->workspaces[i].monitor);
        for (int j = 0; j < parsed->workspaces[i].rule_count; j++) {
            free(parsed->workspaces[i].rules[j]);
        }
        free(parsed->workspaces[i].rules);
    }
    free(parsed->workspaces);
    
    for (int i = 0; i < parsed->keybind_count; i++) {
        free(parsed->keybinds[i].key_combo);
        free(parsed->keybinds[i].command);
        free(parsed->keybinds[i].wm_exclusive);
    }
    free(parsed->keybinds);
    
    free(parsed);
}

int wmswitch_config_validate(wmswitch_parsed_config_t *config) {
    if (!config) {
        wmswitch_log_error("Cannot validate NULL config");
        return 0;
    }
    
    if (config->workspace_count == 0) {
        wmswitch_log_warning("No workspaces defined in config");
    }
    
    if (config->keybind_count == 0) {
        wmswitch_log_warning("No keybinds defined in config");
    }
    
    for (int i = 0; i < config->workspace_count; i++) {
        if (!config->workspaces[i].name || strlen(config->workspaces[i].name) == 0) {
            wmswitch_log_error("Workspace %d has invalid name", i);
            return 0;
        }
    }
    
    wmswitch_log_info("Config validation passed");
    return 1;
}