#include "wewe_config.h"
#include <cyaml/cyaml.h>
#include <stdio.h>
#include <syslog.h> // For syslog

// Schemas must be defined from the inside out.

// 1. Schema for the fields of a single network object.
static const cyaml_schema_field_t network_fields_schema[] = {
    CYAML_FIELD_STRING_PTR("name", CYAML_FLAG_POINTER, network_t, name, 0, CYAML_UNLIMITED),
    CYAML_FIELD_STRING_PTR("gateway", CYAML_FLAG_POINTER, network_t, gateway, 0, CYAML_UNLIMITED),
    CYAML_FIELD_END
};

// 2. Schema for the value of a single network object (used in sequences).
static const cyaml_schema_value_t network_value_schema = {
    CYAML_VALUE_MAPPING(CYAML_FLAG_DEFAULT, network_t, network_fields_schema),
};

// 3. Schema for the fields of a single user object.
static const cyaml_schema_field_t user_fields_schema[] = {
    CYAML_FIELD_STRING_PTR("username", CYAML_FLAG_POINTER, user_t, username, 0, CYAML_UNLIMITED),
    CYAML_FIELD_STRING_PTR("pin_hash", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL, user_t, pin_hash, 0, CYAML_UNLIMITED),
    CYAML_FIELD_BOOL("trusted_network_check", 0, user_t, trusted_network_check),
    // The sequence entry schema must be a pointer to a cyaml_schema_value_t.
    CYAML_FIELD_SEQUENCE("networks", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL, user_t, networks, &network_value_schema, 0, CYAML_UNLIMITED),
    CYAML_FIELD_END
};

// 4. Schema for the value of a single user object (used in sequences).
static const cyaml_schema_value_t user_value_schema = {
    CYAML_VALUE_MAPPING(CYAML_FLAG_DEFAULT, user_t, user_fields_schema),
};

// 5. Schema for the top-level fields of the document.
static const cyaml_schema_field_t top_level_field_schema[] = {
    CYAML_FIELD_SEQUENCE("users", CYAML_FLAG_POINTER, config_t, users, &user_value_schema, 0, CYAML_UNLIMITED),
    CYAML_FIELD_END
};

// 6. Schema for the top-level value of the document.
static const cyaml_schema_value_t top_schema = {
    CYAML_VALUE_MAPPING(CYAML_FLAG_POINTER, config_t, top_level_field_schema),
};

// CYAML config object
static const cyaml_config_t cyaml_conf = {
    .log_fn = cyaml_log,
    .mem_fn = cyaml_mem,
    .log_level = CYAML_LOG_WARNING,
};

config_t* load_config_from_file(const char *filepath) {
    config_t *loaded_config = NULL;
    cyaml_err_t err;

    openlog("pam_wewe_c", LOG_PID, LOG_AUTH);

    err = cyaml_load_file(filepath, &cyaml_conf, &top_schema, (void **)&loaded_config, NULL);
    if (err != CYAML_OK) {
        syslog(LOG_ERR, "Failed to load YAML file %s: %s", filepath, cyaml_strerror(err));
        closelog();
        return NULL;
    }

    syslog(LOG_INFO, "Successfully loaded config from %s", filepath);
    closelog();
    return loaded_config;
}

void free_config(config_t *config) {
    cyaml_free(&cyaml_conf, &top_schema, config, 0);
}