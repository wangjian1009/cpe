#include "cpe/pal/pal_strings.h"
#include "zip_pro_defs.h"

#ifdef CASESENSITIVITYDEFAULT_NO
#define CASESENSITIVITYDEFAULTVALUE 2
#else
#define CASESENSITIVITYDEFAULTVALUE 1
#endif

/*
   Compare two filename (fileName1,fileName2).
   If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
   If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
                                                                or strcasecmp)
   If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
        (like 1 on Unix, 2 on Windows)

*/
extern int cpe_zip_fname_cmp(
    const char * fileName1, const char * fileName2, int iCaseSensitivity)
{
    if (iCaseSensitivity == 0)
        iCaseSensitivity = CASESENSITIVITYDEFAULTVALUE;

    if (iCaseSensitivity == 1)
        return strcmp(fileName1, fileName2);

    return strcasecmp(fileName1, fileName2);
}


