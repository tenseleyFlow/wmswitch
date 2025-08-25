#ifndef WMSWITCH_VALIDATOR_H
#define WMSWITCH_VALIDATOR_H

#include "toml_config.h"

typedef enum {
    WMSWITCH_VALIDATION_OK,
    WMSWITCH_VALIDATION_WARNING,
    WMSWITCH_VALIDATION_ERROR
} wmswitch_validation_level_t;

typedef struct {
    wmswitch_validation_level_t level;
    char *message;
    char *context;
    int line_number;
} wmswitch_validation_issue_t;

typedef struct {
    wmswitch_validation_issue_t *issues;
    int issue_count;
    int capacity;
} wmswitch_validation_report_t;

wmswitch_validation_report_t* wmswitch_validate_config_advanced(wmswitch_parsed_config_t *config);
void wmswitch_validation_report_free(wmswitch_validation_report_t *report);

int wmswitch_detect_keybind_conflicts(wmswitch_parsed_config_t *config, wmswitch_validation_report_t *report);
int wmswitch_detect_workspace_conflicts(wmswitch_parsed_config_t *config, wmswitch_validation_report_t *report);
int wmswitch_validate_monitor_assignments(wmswitch_parsed_config_t *config, wmswitch_validation_report_t *report);

void wmswitch_print_validation_report(wmswitch_validation_report_t *report);

#endif