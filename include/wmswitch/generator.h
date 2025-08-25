#ifndef WMSWITCH_GENERATOR_H
#define WMSWITCH_GENERATOR_H

#include "toml_config.h"

typedef enum {
    WMSWITCH_WM_I3,
    WMSWITCH_WM_HYPRLAND,
    WMSWITCH_WM_AEROSPACE,
    WMSWITCH_WM_COUNT
} wmswitch_wm_type_t;

typedef struct {
    wmswitch_wm_type_t type;
    const char *name;
    const char *config_path;
    const char *config_filename;
    int (*generate)(wmswitch_parsed_config_t *config, const char *output_path, int dry_run);
    int (*backup)(const char *config_path);
    int (*validate_generated)(const char *config_path);
} wmswitch_generator_t;

typedef struct {
    char *config_dir;
    char *backup_dir;
    int create_backups;
    int dry_run;
    wmswitch_wm_type_t *target_wms;
    int target_wm_count;
} wmswitch_generation_options_t;

wmswitch_generator_t* wmswitch_get_generators(void);
wmswitch_generator_t* wmswitch_get_generator_by_type(wmswitch_wm_type_t type);
wmswitch_generator_t* wmswitch_get_generator_by_name(const char *name);

int wmswitch_generate_all_configs(wmswitch_parsed_config_t *config, wmswitch_generation_options_t *options);

char* wmswitch_get_xdg_config_home(void);
char* wmswitch_get_config_path_for_wm(wmswitch_wm_type_t wm_type);

wmswitch_generation_options_t* wmswitch_generation_options_create(void);
void wmswitch_generation_options_free(wmswitch_generation_options_t *options);

#endif