#include <assert.h>
#include "pcre2.h"
#include "cmocka_all.h"

void _assert_string_match(
    const char * const check, const char * const regex,
    const char * const file, const int line)
{
    int reerr;
    PCRE2_SIZE reerroffset;
    pcre2_code * re = pcre2_compile_8((PCRE2_SPTR8)regex, PCRE2_ZERO_TERMINATED, 0, &reerr, &reerroffset, NULL);
    if (re == NULL) {
        print_error("regex \"%s\" error, failed: %d, offset %d", regex, reerr, (int)reerroffset);
        _fail(file, line);
    }

    pcre2_match_data * match_data = pcre2_match_data_create_from_pattern(re, NULL);
    int rc = pcre2_match_8(re, (PCRE2_SPTR8)check, PCRE2_ZERO_TERMINATED, 0, 0, match_data, NULL);
    pcre2_match_data_free(match_data);
    
    pcre2_code_free(re);

    if (rc < 0) {
        print_error("expect \"%s\" match \"%s\" failed", check, regex);
        _fail(file, line);
    }
}

int cpe_cmocak_run_test_suits(const struct _CPE_TEST_SUIT * test_suits, uint16_t test_suit_count) {
    uint32_t total_count = 0;

    uint16_t i;
    for(i = 0; i < test_suit_count; ++i) {
        total_count += test_suits[i].m_count;
    }

    struct CMUnitTest * all_cases = malloc(sizeof(struct CMUnitTest) * total_count);

    uint32_t case_pos = 0;
    for(i = 0; i < test_suit_count; ++i) {
        uint16_t j;
        for(j = 0; j < test_suits[i].m_count; ++j) {
            all_cases[case_pos++] = test_suits[i].m_cases[j];
        }
    }
    assert(case_pos == total_count);
    
 	int rv = _cmocka_run_group_tests("aa", all_cases, total_count, NULL, NULL);

    free(all_cases);

    return rv;
}
