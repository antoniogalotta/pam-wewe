#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h> // For strcasecmp
#include <argon2.h>
#include "wewe_config.h"
#include "wewe_net.h"

// Declare pam_set_item if not already in headers
extern int pam_set_item(pam_handle_t *pamh, int item_type, const void *item);


PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char *pam_user = NULL;
    char *pin = NULL;
    int pam_err;

    openlog("pam_wewe", LOG_PID, LOG_AUTH);

    const char *config_path = "/etc/wewe/config.yaml"; // Default path

    for (int i = 0; i < argc; ++i) {
        if (strncmp(argv[i], "config_path=", 12) == 0) {
            config_path = argv[i] + 12;
            break;
        }
    }

    pam_err = pam_get_user(pamh, &pam_user, NULL);
    if (pam_err != PAM_SUCCESS || pam_user == NULL) {
        syslog(LOG_ERR, "Could not get username.");
        closelog();
        return PAM_USER_UNKNOWN;
    }

    config_t *config = load_config_from_file(config_path);
    if (config == NULL) {
        syslog(LOG_ERR, "Failed to load wewe config.");
        closelog();
        return PAM_IGNORE; // Fail open
    }

    user_t *user_config = NULL;
    for (size_t i = 0; i < config->users_count; i++) {
        if (strcmp(config->users[i].username, pam_user) == 0) {
            user_config = &config->users[i];
            break;
        }
    }

    if (user_config == NULL || user_config->pin_hash == NULL || strlen(user_config->pin_hash) == 0) {
        syslog(LOG_INFO, "User %s not in wewe config or no PIN, ignoring.", pam_user);
        free_config(config);
        closelog();
        return PAM_IGNORE;
    }

    if (user_config->trusted_network_check) {
        char *gateway_mac = get_default_gateway_mac();
        if (gateway_mac == NULL) {
            syslog(LOG_WARNING, "Could not get gateway MAC, ignoring wewe auth.");
            free_config(config);
            closelog();
            return PAM_IGNORE;
        }
        bool trusted = false;
        for (size_t i = 0; i < user_config->networks_count; i++) {
            if (strcasecmp(gateway_mac, user_config->networks[i].gateway) == 0) {
                trusted = true;
                break;
            }
        }
        free(gateway_mac);
        if (!trusted) {
            syslog(LOG_INFO, "User %s not on a trusted network, ignoring wewe auth.", pam_user);
            free_config(config);
        closelog();
            return PAM_IGNORE;
        }
    }

    pam_err = pam_get_authtok(pamh, PAM_AUTHTOK, (const char **)&pin, "PIN: ");
    if (pam_err != PAM_SUCCESS) {
        syslog(LOG_ERR, "pam_get_authtok failed: %s", pam_strerror(pamh, pam_err));
        free_config(config);
        closelog();
        return PAM_AUTH_ERR;
    }

    if (argon2id_verify(user_config->pin_hash, pin, strlen(pin)) != ARGON2_OK) {
        syslog(LOG_WARNING, "Invalid PIN for user %s.", pam_user);
        pam_set_item(pamh, PAM_AUTHTOK, NULL); // Clear authtok
        free_config(config);
        closelog();
        return PAM_AUTH_ERR;
    }

    syslog(LOG_INFO, "User %s authenticated successfully via wewe.", pam_user);
    free_config(config);
    closelog();
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}
