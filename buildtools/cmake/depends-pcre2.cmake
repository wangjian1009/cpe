set(pcre2_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/pcre2)

add_definitions(
  -DPCRE2_CODE_UNIT_WIDTH=8
  -DHAVE_CONFIG_H
  -Wno-implicit-function-declaration -O3
  )

include_directories(
  ${pcre2_base}/include
  )

set(pcre2_source
  ${pcre2_base}/src/pcre2_auto_possess.c
  ${pcre2_base}/src/pcre2_chartables.c
  ${pcre2_base}/src/pcre2_compile.c
  ${pcre2_base}/src/pcre2_config.c
  ${pcre2_base}/src/pcre2_context.c
  ${pcre2_base}/src/pcre2_dfa_match.c
  ${pcre2_base}/src/pcre2_error.c
  ${pcre2_base}/src/pcre2_find_bracket.c 
  ${pcre2_base}/src/pcre2_jit_compile.c
  ${pcre2_base}/src/pcre2_maketables.c
  ${pcre2_base}/src/pcre2_match.c
  ${pcre2_base}/src/pcre2_match_data.c
  ${pcre2_base}/src/pcre2_newline.c
  ${pcre2_base}/src/pcre2_ord2utf.c
  ${pcre2_base}/src/pcre2_pattern_info.c
  ${pcre2_base}/src/pcre2_serialize.c
  ${pcre2_base}/src/pcre2_string_utils.c
  ${pcre2_base}/src/pcre2_study.c
  ${pcre2_base}/src/pcre2_substitute.c
  ${pcre2_base}/src/pcre2_substring.c
  ${pcre2_base}/src/pcre2_tables.c
  ${pcre2_base}/src/pcre2_ucd.c
  ${pcre2_base}/src/pcre2_valid_utf.c
  ${pcre2_base}/src/pcre2_xclass.c
  )

add_library(pcre2 STATIC ${pcre2_source})
