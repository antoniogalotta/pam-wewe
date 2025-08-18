#ifndef WEWE_CONFIG_H
#define WEWE_CONFIG_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char *name;
    char *gateway;
} network_t;

typedef struct {
    char *username;
    char *pin_hash;
    bool trusted_network_check;
    network_t *networks;
    size_t networks_count;
} user_t;

typedef struct {
    user_t *users;
    size_t users_count;
} config_t;

config_t* load_config_from_file(const char *filepath);
void free_config(config_t *config);

#endif // WEWE_CONFIG_H
