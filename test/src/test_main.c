#include <check.h>
#include <stdlib.h>

Suite *pam_wewe_suite(void);
Suite *wewe_config_files_suite(void);

int main(void)
{
    int number_failed;
    SRunner *sr;

    sr = srunner_create(pam_wewe_suite());
    srunner_add_suite(sr, wewe_config_files_suite());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
