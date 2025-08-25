#ifndef WMSWITCH_INHERITANCE_H
#define WMSWITCH_INHERITANCE_H

#include "toml_config.h"

typedef struct {
    char **include_paths;
    int include_count;
    char *base_dir;
} wmswitch_include_context_t;

wmswitch_config_t* wmswitch_config_load_with_includes(const char *filepath);
int wmswitch_process_includes(wmswitch_config_t *config, wmswitch_include_context_t *context);
wmswitch_parsed_config_t* wmswitch_merge_configs(wmswitch_parsed_config_t *base, wmswitch_parsed_config_t *override);

void wmswitch_include_context_free(wmswitch_include_context_t *context);

#endif