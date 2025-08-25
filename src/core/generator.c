#include "wmswitch/generator.h"
#include "wmswitch/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int wmswitch_i3_generate(wmswitch_parsed_config_t *config, const char *output_path, int dry_run);
int wmswitch_hyprland_generate(wmswitch_parsed_config_t *config, const char *output_path, int dry_run);
int wmswitch_aerospace_generate(wmswitch_parsed_config_t *config, const char *output_path, int dry_run);

int wmswitch_backup_config(const char *config_path);
int wmswitch_validate_i3_config(const char *config_path);
int wmswitch_validate_hyprland_config(const char *config_path);
int wmswitch_validate_aerospace_config(const char *config_path);

static wmswitch_generator_t generators[] = {
    {
        .type = WMSWITCH_WM_I3,
        .name = "i3",
        .config_path = "i3",
        .config_filename = "config",
        .generate = wmswitch_i3_generate,
        .backup = wmswitch_backup_config,
        .validate_generated = wmswitch_validate_i3_config
    },
    {
        .type = WMSWITCH_WM_HYPRLAND,
        .name = "hyprland", 
        .config_path = "hypr",
        .config_filename = "hyprland.conf",
        .generate = wmswitch_hyprland_generate,
        .backup = wmswitch_backup_config,
        .validate_generated = wmswitch_validate_hyprland_config
    },
    {
        .type = WMSWITCH_WM_AEROSPACE,
        .name = "aerospace",
        .config_path = "aerospace",
        .config_filename = "aerospace.toml",
        .generate = wmswitch_aerospace_generate,
        .backup = wmswitch_backup_config,
        .validate_generated = wmswitch_validate_aerospace_config
    }
};

wmswitch_generator_t* wmswitch_get_generators(void) {
    return generators;
}

wmswitch_generator_t* wmswitch_get_generator_by_type(wmswitch_wm_type_t type) {
    if (type >= WMSWITCH_WM_COUNT) {
        return NULL;
    }
    return &generators[type];
}

wmswitch_generator_t* wmswitch_get_generator_by_name(const char *name) {
    if (!name) return NULL;
    
    for (int i = 0; i < WMSWITCH_WM_COUNT; i++) {
        if (strcmp(generators[i].name, name) == 0) {
            return &generators[i];
        }
    }
    return NULL;
}

char* wmswitch_get_xdg_config_home(void) {
    const char *xdg_config = getenv("XDG_CONFIG_HOME");
    if (xdg_config) {
        return strdup(xdg_config);
    }
    
    const char *home = getenv("HOME");
    if (!home) {
        wmswitch_log_error("Cannot determine home directory");
        return NULL;
    }
    
    char *config_home = malloc(strlen(home) + strlen("/.config") + 1);
    sprintf(config_home, "%s/.config", home);
    return config_home;
}

char* wmswitch_get_config_path_for_wm(wmswitch_wm_type_t wm_type) {
    wmswitch_generator_t *generator = wmswitch_get_generator_by_type(wm_type);
    if (!generator) {
        return NULL;
    }
    
    char *config_home = wmswitch_get_xdg_config_home();
    if (!config_home) {
        return NULL;
    }
    
    char *config_path = malloc(strlen(config_home) + strlen(generator->config_path) + 
                              strlen(generator->config_filename) + 3);
    sprintf(config_path, "%s/%s/%s", config_home, generator->config_path, generator->config_filename);
    
    free(config_home);
    return config_path;
}

static int ensure_directory_exists(const char *path) {
    struct stat st = {0};
    
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            wmswitch_log_error("Failed to create directory: %s", path);
            return 0;
        }
        wmswitch_log_debug("Created directory: %s", path);
    }
    
    return 1;
}

int wmswitch_backup_config(const char *config_path) {
    if (access(config_path, F_OK) != 0) {
        wmswitch_log_debug("No existing config to backup at: %s", config_path);
        return 1;
    }
    
    char backup_path[PATH_MAX];
    snprintf(backup_path, sizeof(backup_path), "%s.wmswitch.backup", config_path);
    
    FILE *src = fopen(config_path, "r");
    if (!src) {
        wmswitch_log_error("Failed to open source config for backup: %s", config_path);
        return 0;
    }
    
    FILE *dst = fopen(backup_path, "w");
    if (!dst) {
        fclose(src);
        wmswitch_log_error("Failed to create backup file: %s", backup_path);
        return 0;
    }
    
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes, dst) != bytes) {
            fclose(src);
            fclose(dst);
            wmswitch_log_error("Failed to write backup file: %s", backup_path);
            return 0;
        }
    }
    
    fclose(src);
    fclose(dst);
    
    wmswitch_log_info("Created backup: %s", backup_path);
    return 1;
}

int wmswitch_generate_all_configs(wmswitch_parsed_config_t *config, wmswitch_generation_options_t *options) {
    if (!config || !options) {
        wmswitch_log_error("Invalid parameters for config generation");
        return 0;
    }
    
    int success_count = 0;
    int total_count = options->target_wm_count > 0 ? options->target_wm_count : WMSWITCH_WM_COUNT;
    
    for (int i = 0; i < total_count; i++) {
        wmswitch_wm_type_t wm_type = options->target_wm_count > 0 ? 
            options->target_wms[i] : (wmswitch_wm_type_t)i;
            
        wmswitch_generator_t *generator = wmswitch_get_generator_by_type(wm_type);
        if (!generator) {
            wmswitch_log_warning("No generator found for WM type: %d", wm_type);
            continue;
        }
        
        char *config_path = wmswitch_get_config_path_for_wm(wm_type);
        if (!config_path) {
            wmswitch_log_error("Failed to determine config path for %s", generator->name);
            continue;
        }
        
        char *config_dir = strdup(config_path);
        char *last_slash = strrchr(config_dir, '/');
        if (last_slash) {
            *last_slash = '\0';
        }
        
        if (!ensure_directory_exists(config_dir)) {
            free(config_path);
            free(config_dir);
            continue;
        }
        
        if (options->create_backups && !options->dry_run) {
            if (!generator->backup(config_path)) {
                wmswitch_log_warning("Failed to backup existing config for %s", generator->name);
            }
        }
        
        wmswitch_log_info("Generating %s config...", generator->name);
        if (generator->generate(config, config_path, options->dry_run)) {
            success_count++;
            wmswitch_log_info("Successfully generated %s config", generator->name);
        } else {
            wmswitch_log_error("Failed to generate %s config", generator->name);
        }
        
        free(config_path);
        free(config_dir);
    }
    
    wmswitch_log_info("Generated %d/%d configs successfully", success_count, total_count);
    return success_count == total_count;
}

wmswitch_generation_options_t* wmswitch_generation_options_create(void) {
    wmswitch_generation_options_t *options = calloc(1, sizeof(wmswitch_generation_options_t));
    if (!options) {
        return NULL;
    }
    
    options->config_dir = wmswitch_get_xdg_config_home();
    options->create_backups = 1;
    options->dry_run = 0;
    options->target_wms = NULL;
    options->target_wm_count = 0;
    
    return options;
}

void wmswitch_generation_options_free(wmswitch_generation_options_t *options) {
    if (!options) return;
    
    free(options->config_dir);
    free(options->backup_dir);
    free(options->target_wms);
    free(options);
}