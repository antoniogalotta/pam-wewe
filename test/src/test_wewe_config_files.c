#include <check.h>
#include <stdlib.h>
#include <string.h>
#include "wewe_config.h"

START_TEST(test_config_load)
{
    config_t *config = load_config_from_file("test/resources/config.yaml");
    ck_assert_ptr_ne(config, NULL);
    ck_assert_int_eq(config->users_count, 4);

    // User 1: test
    ck_assert_str_eq(config->users[0].username, "test");
    ck_assert_str_eq(config->users[0].pin_hash, "$argon2id$v=19$m=19456,t=2,p=1$5wsSTJK0/CL03/eXED707Q$eOKG0yBSs0zXaLilSkdcuI54yuRrJ3Zyft4Asm5LMak");
    ck_assert_int_eq(config->users[0].networks_count, 1);
    ck_assert_str_eq(config->users[0].networks[0].name, "home");
    ck_assert_str_eq(config->users[0].networks[0].gateway, "00:aa:0a:a0:21:12");
    ck_assert_int_eq(config->users[0].trusted_network_check, 1);

    // User 2: test2
    ck_assert_str_eq(config->users[1].username, "test2");
    ck_assert_str_eq(config->users[1].pin_hash, "$argon2id$v=19$m=19456,t=2,p=1$5wsSTJK0/CL03/eXED707Q$eOKG0yBSs0zXaLilSkdcuI54yuRrJ3Zyft4Asm5LMak");
    ck_assert_int_eq(config->users[1].networks_count, 1);
    ck_assert_str_eq(config->users[1].networks[0].name, "home");
    ck_assert_str_eq(config->users[1].networks[0].gateway, "00:aa:0a:a0:21:12");
    ck_assert_int_eq(config->users[1].trusted_network_check, 0);

    // User 3: test3
    ck_assert_str_eq(config->users[2].username, "test3");
    ck_assert_str_eq(config->users[2].pin_hash, "$argon2id$v=19$m=19456,t=2,p=1$5wsSTJK0/CL03/eXED707Q$eOKG0yBSs0zXaLilSkdcuI54yuRrJ3Zyft4Asm5LMak");
    ck_assert_int_eq(config->users[2].networks_count, 0);
    ck_assert_int_eq(config->users[2].trusted_network_check, 1);

    // User 4: test4
    ck_assert_str_eq(config->users[3].username, "test4");
    ck_assert_str_eq(config->users[3].pin_hash, "$argon2id$v=19$m=19456,t=2,p=1$5wsSTJK0/CL03/eXED707Q$eOKG0yBSs0zXaLilSkdcuI54yuRrJ3Zyft4Asm5LMak");
    ck_assert_int_eq(config->users[3].networks_count, 0);
    ck_assert_int_eq(config->users[3].trusted_network_check, 0);

    free_config(config);
}
END_TEST

START_TEST(test_config_empty_load)
{
    config_t *config = load_config_from_file("test/resources/config_empty.yaml");
    ck_assert_ptr_ne(config, NULL);
    ck_assert_int_eq(config->users_count, 0);
    free_config(config);
}
END_TEST

Suite *wewe_config_files_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("WeweConfigFiles");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_config_load);
    tcase_add_test(tc_core, test_config_empty_load);
    suite_add_tcase(s, tc_core);

    return s;
}


