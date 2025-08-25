#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include "wmswitch/logging.h"
#include "wmswitch/toml_config.h"
#include "wmswitch/generator.h"
#include "wmswitch/wizard.h"

#define WMSWITCH_VERSION "0.1.0"

static void print_usage(const char *program_name) {
    printf("wmswitch v%s - Unified configuration manager for tiling window managers\n\n", WMSWITCH_VERSION);
    printf("Usage: %s [options] <command> [arguments]\n\n", program_name);
    printf("Commands:\n");
    printf("  generate <config_file>    Generate WM configs from TOML file\n");
    printf("  validate <config_file>    Validate TOML configuration file\n");
    printf("  wizard [output_file]      Interactive configuration wizard\n");
    printf("  version                   Show version information\n\n");
    printf("Options:\n");
    printf("  -h, --help               Show this help message\n");
    printf("  -v, --verbose            Enable verbose output\n");
    printf("  -q, --quiet              Suppress non-error output\n");
    printf("  -d, --debug              Enable debug output\n");
    printf("  --dry-run               Preview changes without writing files\n");
    printf("  --backup                Create backups of existing configs\n\n");
    printf("Examples:\n");
    printf("  %s wizard ~/.config/wmswitch/config.toml\n", program_name);
    printf("  %s generate ~/.config/wmswitch/config.toml\n", program_name);
    printf("  %s validate ~/.config/wmswitch/config.toml\n", program_name);
    printf("  %s --dry-run generate config.toml\n", program_name);
}

static void print_version(void) {
    printf("wmswitch version %s\n", WMSWITCH_VERSION);
    printf("Built with tomlc99 TOML parser\n");
}

static int validate_config_file(const char *config_path) {
    wmswitch_log_info("Validating config file: %s", config_path);
    
    if (access(config_path, R_OK) != 0) {
        wmswitch_log_error("Cannot read config file: %s", config_path);
        return 1;
    }
    
    wmswitch_config_t *config = wmswitch_config_load(config_path);
    if (!config) {
        return 1;
    }
    
    wmswitch_parsed_config_t *parsed = wmswitch_config_parse(config);
    if (!parsed) {
        wmswitch_config_free(config);
        return 1;
    }
    
    int valid = wmswitch_config_validate(parsed);
    
    wmswitch_parsed_config_free(parsed);
    wmswitch_config_free(config);
    
    if (valid) {
        wmswitch_log_info("Configuration is valid!");
        return 0;
    } else {
        wmswitch_log_error("Configuration validation failed");
        return 1;
    }
}

static int generate_configs(const char *config_path, int dry_run, int create_backup) {
    wmswitch_log_info("Generating configs from: %s", config_path);
    
    if (dry_run) {
        wmswitch_log_info("DRY RUN MODE - no files will be modified");
    }
    
    if (create_backup) {
        wmswitch_log_info("Backup mode enabled");
    }
    
    wmswitch_config_t *config = wmswitch_config_load(config_path);
    if (!config) {
        return 1;
    }
    
    wmswitch_parsed_config_t *parsed = wmswitch_config_parse(config);
    if (!parsed) {
        wmswitch_config_free(config);
        return 1;
    }
    
    if (!wmswitch_config_validate(parsed)) {
        wmswitch_parsed_config_free(parsed);
        wmswitch_config_free(config);
        return 1;
    }
    
    wmswitch_generation_options_t *options = wmswitch_generation_options_create();
    if (!options) {
        wmswitch_log_error("Failed to create generation options");
        wmswitch_parsed_config_free(parsed);
        wmswitch_config_free(config);
        return 1;
    }
    
    options->dry_run = dry_run;
    options->create_backups = create_backup;
    
    int success = wmswitch_generate_all_configs(parsed, options);
    
    wmswitch_generation_options_free(options);
    wmswitch_parsed_config_free(parsed);
    wmswitch_config_free(config);
    
    return success ? 0 : 1;
}

int main(int argc, char *argv[]) {
    wmswitch_log_init(WMSWITCH_LOG_INFO);
    
    int opt;
    int dry_run = 0;
    int create_backup = 0;
    
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"verbose", no_argument, 0, 'v'},
        {"quiet", no_argument, 0, 'q'},
        {"debug", no_argument, 0, 'd'},
        {"dry-run", no_argument, 0, 1001},
        {"backup", no_argument, 0, 1002},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "hvqd", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'v':
                wmswitch_log_set_level(WMSWITCH_LOG_INFO);
                break;
            case 'q':
                wmswitch_log_set_level(WMSWITCH_LOG_ERROR);
                break;
            case 'd':
                wmswitch_log_set_level(WMSWITCH_LOG_DEBUG);
                break;
            case 1001:
                dry_run = 1;
                break;
            case 1002:
                create_backup = 1;
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    if (optind >= argc) {
        fprintf(stderr, "Error: Missing command\n");
        print_usage(argv[0]);
        return 1;
    }
    
    const char *command = argv[optind];
    
    if (strcmp(command, "version") == 0) {
        print_version();
        return 0;
    }
    
    if (strcmp(command, "help") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    if (strcmp(command, "wizard") == 0) {
        const char *output_path = "wmswitch_config.toml";
        if (optind + 1 < argc) {
            output_path = argv[optind + 1];
        }
        return wmswitch_run_wizard(output_path) ? 0 : 1;
    }
    
    if (optind + 1 >= argc) {
        fprintf(stderr, "Error: Missing config file argument for command '%s'\n", command);
        print_usage(argv[0]);
        return 1;
    }
    
    const char *config_path = argv[optind + 1];
    
    if (strcmp(command, "validate") == 0) {
        return validate_config_file(config_path);
    } else if (strcmp(command, "generate") == 0) {
        return generate_configs(config_path, dry_run, create_backup);
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n", command);
        print_usage(argv[0]);
        return 1;
    }
}