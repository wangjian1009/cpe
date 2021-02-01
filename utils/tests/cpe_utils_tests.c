#include "cmocka_all.h"
#include "cpe_utils_tests.h"

int main(void) {
    int rv = 0;

    /*basic*/
    if (cpe_url_basic_tests() != 0) rv = -1;

    return rv;
}


