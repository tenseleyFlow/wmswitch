#define _GNU_SOURCE
#include "wmswitch/validator.h"
#include "wmswitch/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void add_validation_issue(wmswitch_validation_report_t *report, 
                                wmswitch_validation_level_t level,
                                const char *message, const char *context) {
    if (report->issue_count >= report->capacity) {
        report->capacity = report->capacity == 0 ? 10 : report->capacity * 2;
        report->issues = realloc(report->issues, 
                                report->capacity * sizeof(wmswitch_validation_issue_t));
    }
    
    wmswitch_validation_issue_t *issue = &report->issues[report->issue_count++];
    issue->level = level;
    issue->message = strdup(message);
    issue->context = context ? strdup(context) : NULL;
    issue->line_number = -1;
}

wmswitch_validation_report_t* wmswitch_validate_config_advanced(wmswitch_parsed_config_t *config) {
    wmswitch_validation_report_t *report = calloc(1, sizeof(wmswitch_validation_report_t));
    if (!report) return NULL;
    
    if (!config) {
        add_validation_issue(report, WMSWITCH_VALIDATION_ERROR, 
                           "Configuration is NULL", NULL);
        return report;
    }
    
    // Basic validation
    if (config->workspace_count == 0) {
        add_validation_issue(report, WMSWITCH_VALIDATION_WARNING,
                           "No workspaces defined", "Consider adding workspace definitions");
    }
    
    if (config->keybind_count == 0) {
        add_validation_issue(report, WMSWITCH_VALIDATION_WARNING,
                           "No keybinds defined", "Consider adding keybind definitions");
    }
    
    // Advanced validation checks
    wmswitch_detect_keybind_conflicts(config, report);
    wmswitch_detect_workspace_conflicts(config, report);
    wmswitch_validate_monitor_assignments(config, report);
    
    return report;
}

int wmswitch_detect_keybind_conflicts(wmswitch_parsed_config_t *config, wmswitch_validation_report_t *report) {
    int conflicts = 0;
    
    for (int i = 0; i < config->keybind_count; i++) {
        for (int j = i + 1; j < config->keybind_count; j++) {
            wmswitch_keybind_t *kb1 = &config->keybinds[i];
            wmswitch_keybind_t *kb2 = &config->keybinds[j];
            
            if (strcmp(kb1->key_combo, kb2->key_combo) == 0) {
                // Same keybind - check if they're WM-exclusive
                if (!kb1->wm_exclusive && !kb2->wm_exclusive) {
                    char message[200];
                    snprintf(message, sizeof(message), 
                            "Keybind conflict: '%s' is defined multiple times", 
                            kb1->key_combo);
                    add_validation_issue(report, WMSWITCH_VALIDATION_ERROR, message, 
                                       "Consider using WM-exclusive sections or removing duplicate");
                    conflicts++;
                } else if (kb1->wm_exclusive && kb2->wm_exclusive && 
                          strcmp(kb1->wm_exclusive, kb2->wm_exclusive) == 0) {
                    char message[200];
                    snprintf(message, sizeof(message), 
                            "Keybind conflict in %s: '%s' is defined multiple times", 
                            kb1->wm_exclusive, kb1->key_combo);
                    add_validation_issue(report, WMSWITCH_VALIDATION_ERROR, message, NULL);
                    conflicts++;
                }
            }
        }
    }
    
    // Check for problematic keybind patterns
    for (int i = 0; i < config->keybind_count; i++) {
        wmswitch_keybind_t *kb = &config->keybinds[i];
        
        // Warn about potentially problematic keybinds
        if (strstr(kb->key_combo, "alt+f4")) {
            add_validation_issue(report, WMSWITCH_VALIDATION_WARNING,
                               "Alt+F4 keybind may conflict with system shortcuts", 
                               kb->key_combo);
        }
        
        if (strstr(kb->key_combo, "ctrl+alt+")) {
            add_validation_issue(report, WMSWITCH_VALIDATION_WARNING,
                               "Ctrl+Alt combinations may conflict with system shortcuts", 
                               kb->key_combo);
        }
    }
    
    return conflicts;
}

int wmswitch_detect_workspace_conflicts(wmswitch_parsed_config_t *config, wmswitch_validation_report_t *report) {
    int conflicts = 0;
    
    for (int i = 0; i < config->workspace_count; i++) {
        for (int j = i + 1; j < config->workspace_count; j++) {
            wmswitch_workspace_t *ws1 = &config->workspaces[i];
            wmswitch_workspace_t *ws2 = &config->workspaces[j];
            
            // Check for duplicate workspace numbers
            if (ws1->number > 0 && ws2->number > 0 && ws1->number == ws2->number) {
                char message[200];
                snprintf(message, sizeof(message), 
                        "Workspace number conflict: Both '%s' and '%s' use number %d", 
                        ws1->name, ws2->name, ws1->number);
                add_validation_issue(report, WMSWITCH_VALIDATION_ERROR, message, 
                                   "Workspace numbers must be unique");
                conflicts++;
            }
            
            // Check for duplicate workspace names
            if (strcmp(ws1->name, ws2->name) == 0) {
                char message[200];
                snprintf(message, sizeof(message), 
                        "Workspace name conflict: '%s' is defined multiple times", 
                        ws1->name);
                add_validation_issue(report, WMSWITCH_VALIDATION_ERROR, message, 
                                   "Workspace names must be unique");
                conflicts++;
            }
        }
    }
    
    return conflicts;
}

int wmswitch_validate_monitor_assignments(wmswitch_parsed_config_t *config, wmswitch_validation_report_t *report) {
    int issues = 0;
    
    // Track monitor usage
    char monitors[10][64]; // Support up to 10 different monitor names
    int monitor_count = 0;
    
    for (int i = 0; i < config->workspace_count; i++) {
        wmswitch_workspace_t *ws = &config->workspaces[i];
        
        if (!ws->monitor || strlen(ws->monitor) == 0) {
            char message[200];
            snprintf(message, sizeof(message), 
                    "Workspace '%s' has no monitor assignment", ws->name);
            add_validation_issue(report, WMSWITCH_VALIDATION_WARNING, message, 
                               "Consider assigning workspaces to specific monitors");
            issues++;
            continue;
        }
        
        // Track unique monitors
        int found = 0;
        for (int j = 0; j < monitor_count; j++) {
            if (strcmp(monitors[j], ws->monitor) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found && monitor_count < 10) {
            strncpy(monitors[monitor_count], ws->monitor, sizeof(monitors[0]) - 1);
            monitors[monitor_count][sizeof(monitors[0]) - 1] = '\0';
            monitor_count++;
        }
    }
    
    // Report monitor usage statistics
    if (monitor_count > 0) {
        char message[200];
        snprintf(message, sizeof(message), 
                "Configuration uses %d monitor(s)", monitor_count);
        add_validation_issue(report, WMSWITCH_VALIDATION_OK, message, NULL);
        
        for (int i = 0; i < monitor_count; i++) {
            char monitor_info[200];
            int workspace_count = 0;
            
            // Count workspaces per monitor
            for (int j = 0; j < config->workspace_count; j++) {
                if (config->workspaces[j].monitor && 
                    strcmp(config->workspaces[j].monitor, monitors[i]) == 0) {
                    workspace_count++;
                }
            }
            
            snprintf(monitor_info, sizeof(monitor_info), 
                    "Monitor '%s' has %d workspace(s)", monitors[i], workspace_count);
            add_validation_issue(report, WMSWITCH_VALIDATION_OK, monitor_info, NULL);
        }
    }
    
    return issues;
}

void wmswitch_print_validation_report(wmswitch_validation_report_t *report) {
    if (!report) return;
    
    int errors = 0, warnings = 0, info = 0;
    
    for (int i = 0; i < report->issue_count; i++) {
        wmswitch_validation_issue_t *issue = &report->issues[i];
        
        const char *level_str;
        const char *color;
        
        switch (issue->level) {
            case WMSWITCH_VALIDATION_ERROR:
                level_str = "ERROR";
                color = "\033[31m"; // Red
                errors++;
                break;
            case WMSWITCH_VALIDATION_WARNING:
                level_str = "WARNING";
                color = "\033[33m"; // Yellow
                warnings++;
                break;
            case WMSWITCH_VALIDATION_OK:
                level_str = "INFO";
                color = "\033[32m"; // Green
                info++;
                break;
        }
        
        printf("%s[%s]\033[0m %s", color, level_str, issue->message);
        if (issue->context) {
            printf(" (%s)", issue->context);
        }
        printf("\n");
    }
    
    printf("\nValidation Summary: ");
    if (errors > 0) {
        printf("\033[31m%d error(s)\033[0m, ", errors);
    }
    if (warnings > 0) {
        printf("\033[33m%d warning(s)\033[0m, ", warnings);
    }
    printf("\033[32m%d info message(s)\033[0m\n", info);
    
    if (errors == 0) {
        printf("\n\033[32m✅ Configuration is valid!\033[0m\n");
    } else {
        printf("\n\033[31m❌ Configuration has errors that should be fixed.\033[0m\n");
    }
}

void wmswitch_validation_report_free(wmswitch_validation_report_t *report) {
    if (!report) return;
    
    for (int i = 0; i < report->issue_count; i++) {
        free(report->issues[i].message);
        free(report->issues[i].context);
    }
    
    free(report->issues);
    free(report);
}