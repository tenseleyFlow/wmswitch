#ifndef WMSWITCH_WIZARD_H
#define WMSWITCH_WIZARD_H

typedef struct {
    char *terminal;
    char *app_launcher;
    char *file_manager;
    char *browser;
    int workspace_count;
    char **workspace_names;
    char *primary_monitor;
    char *secondary_monitor;
    int use_vim_bindings;
    int enable_floating;
} wmswitch_wizard_config_t;

int wmswitch_run_wizard(const char *output_path);
void wmswitch_wizard_config_free(wmswitch_wizard_config_t *config);

#endif