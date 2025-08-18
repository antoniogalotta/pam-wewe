#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// --- Mocking PAM ---
#define PAM_SUCCESS 0
#define PAM_AUTH_ERR 7
#define PAM_USER_UNKNOWN 10
#define PAM_IGNORE 25
#define PAM_SERVICE_ERR 4
#define PAM_CONV_ERR 19
#define PAM_AUTHTOK 5
#define PAM_USER 1



#include <argon2.h>

// --- Mocking syslog ---
#include <syslog.h>

// --- Forward declaration of the function to be tested ---
#include <security/pam_modules.h>
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv);

// --- Test Globals & Control ---
static const char* MOCK_PAM_USER = NULL;
static char* MOCK_PIN = NULL;
static int MOCK_PAM_GET_USER_RETURN = PAM_SUCCESS;
static int MOCK_PAM_GET_AUTHTOK_RETURN = PAM_SUCCESS;
static char* MOCK_GATEWAY_MAC = NULL;

static void reset_mocks() {
    MOCK_PAM_USER = NULL;
    MOCK_PIN = NULL;
    MOCK_PAM_GET_USER_RETURN = PAM_SUCCESS;
    MOCK_PAM_GET_AUTHTOK_RETURN = PAM_SUCCESS;
    if (MOCK_GATEWAY_MAC) {
        free(MOCK_GATEWAY_MAC);
        MOCK_GATEWAY_MAC = NULL;
    }
}

// --- Mock Implementations ---
void openlog(const char *ident, int option, int facility) { (void)ident; (void)option; (void)facility; }
void syslog(int priority, const char *format, ...) { (void)priority; (void)format; }
void closelog(void) { }

int pam_get_user(pam_handle_t *pamh, const char **user, const char *prompt) {
    (void)pamh; (void)prompt;
    if (MOCK_PAM_GET_USER_RETURN == PAM_SUCCESS) { *user = MOCK_PAM_USER; }
    return MOCK_PAM_GET_USER_RETURN;
}

int pam_get_authtok(pam_handle_t *pamh, int item_type, const char **authtok, const char *prompt) {
    (void)pamh; (void)item_type; (void)prompt;
    if (MOCK_PAM_GET_AUTHTOK_RETURN == PAM_SUCCESS) { *authtok = MOCK_PIN; }
    return MOCK_PAM_GET_AUTHTOK_RETURN;
}

const char *pam_strerror(pam_handle_t *pamh, int errnum) { (void)pamh; (void)errnum; return "mock pam error"; }
int pam_set_item(pam_handle_t *pamh, int item_type, const void *item) { (void)pamh; (void)item_type; (void)item; return PAM_SUCCESS; }
char *get_default_gateway_mac() { return MOCK_GATEWAY_MAC ? strdup(MOCK_GATEWAY_MAC) : NULL; }

// --- Test Cases ---

START_TEST(test_pam_user_not_in_config)
{
    reset_mocks();
    MOCK_PAM_USER = "unknown_user";
    const char *argv[] = {"config_path=test/resources/config.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_IGNORE);
}
END_TEST

START_TEST(test_pam_config_not_found)
{
    reset_mocks();
    MOCK_PAM_USER = "test";
    const char *argv[] = {"config_path=test/resources/non_existent_config.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_IGNORE);
}
END_TEST

START_TEST(test_pam_empty_config)
{
    reset_mocks();
    MOCK_PAM_USER = "test";
    const char *argv[] = {"config_path=test/resources/config_empty.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_IGNORE);
}
END_TEST

START_TEST(test_pam_get_user_fails)
{
    reset_mocks();
    MOCK_PAM_GET_USER_RETURN = PAM_SERVICE_ERR;
    const char *argv[] = {"config_path=test/resources/config.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_USER_UNKNOWN);
}
END_TEST

START_TEST(test_pam_get_authtok_fails)
{
    reset_mocks();
    MOCK_PAM_USER = "test2";
    MOCK_PAM_GET_AUTHTOK_RETURN = PAM_CONV_ERR;
    const char *argv[] = {"config_path=test/resources/config.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_AUTH_ERR);
}
END_TEST

START_TEST(test_pam_invalid_pin)
{
    reset_mocks();
    MOCK_PAM_USER = "test2";
    MOCK_PIN = "wrong_pin";
    const char *argv[] = {"config_path=test/resources/config.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_AUTH_ERR);
}
END_TEST

START_TEST(test_pam_success_no_network_check)
{
    reset_mocks();
    MOCK_PAM_USER = "test2";
    MOCK_PIN = "0000";
    const char *argv[] = {"config_path=test/resources/config.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_SUCCESS);
}
END_TEST

START_TEST(test_pam_trusted_network_success)
{
    reset_mocks();
    MOCK_PAM_USER = "test";
    MOCK_PIN = "0000";
    MOCK_GATEWAY_MAC = "00:aa:0a:a0:21:12";
    const char *argv[] = {"config_path=test/resources/config.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_SUCCESS);
}
END_TEST

START_TEST(test_pam_trusted_network_fail)
{
    reset_mocks();
    MOCK_PAM_USER = "test";
    MOCK_GATEWAY_MAC = "DE:AD:BE:EF:CA:FE";
    const char *argv[] = {"config_path=test/resources/config.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_IGNORE);
}
END_TEST

START_TEST(test_pam_trusted_network_no_gateway)
{
    reset_mocks();
    MOCK_PAM_USER = "test";
    MOCK_GATEWAY_MAC = NULL;
    const char *argv[] = {"config_path=test/resources/config.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_IGNORE);
}
END_TEST

START_TEST(test_pam_trusted_network_no_config_networks)
{
    reset_mocks();
    MOCK_PAM_USER = "test3";
    MOCK_GATEWAY_MAC = "DE:AD:BE:EF:CA:FE";
    const char *argv[] = {"config_path=test/resources/config.yaml"};
    int result = pam_sm_authenticate(NULL, 0, 1, argv);
    ck_assert_int_eq(result, PAM_IGNORE);
}
END_TEST

Suite *pam_wewe_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("PamWewe");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_pam_user_not_in_config);
    tcase_add_test(tc_core, test_pam_config_not_found);
    tcase_add_test(tc_core, test_pam_empty_config);
    tcase_add_test(tc_core, test_pam_get_user_fails);
    tcase_add_test(tc_core, test_pam_get_authtok_fails);
    tcase_add_test(tc_core, test_pam_invalid_pin);
    tcase_add_test(tc_core, test_pam_success_no_network_check);
    tcase_add_test(tc_core, test_pam_trusted_network_success);
    tcase_add_test(tc_core, test_pam_trusted_network_fail);
    tcase_add_test(tc_core, test_pam_trusted_network_no_gateway);
    tcase_add_test(tc_core, test_pam_trusted_network_no_config_networks);
    suite_add_tcase(s, tc_core);

    return s;
}