// swift-tools-version:5.0

import PackageDescription

let package = Package(
    name: "cpe",
    products: [
        .library(name: "log4c", type: .static, targets: ["log4c"]),
        .library(name: "yajl", type: .static, targets: ["yajl"]),
        .library(name: "pcre2", type: .static, targets: ["pcre2"]),
        .library(name: "cpe_pal", type: .static, targets: ["cpe_pal"]),
        .library(name: "cpe_utils", type: .static, targets: ["cpe_utils"]),
        .library(name: "cpe_utils_sock", type: .static, targets: ["cpe_utils_sock"]),
        .library(name: "cpe_utils_json", type: .static, targets: ["cpe_utils_json"]),
        .library(name: "cpe_vfs", type: .static, targets: ["cpe_vfs"]),
        .library(name: "cpe_fsm", type: .static, targets: ["cpe_fsm"]),
    ],
    targets: [
        .target(name: "cpe_pal"
               , path: "pal"
               , exclude: ["src/msvc_time.c", "src/msvc_dlfcn.c", "src/win32_dirent.c"]
               , sources: ["src"]
        ),
        .target(name: "cpe_utils"
               , path: "utils"
               , exclude: ["src/rwpipe_shm.c"]
               , sources: ["src"]
               , cSettings: [
                     .headerSearchPath("../pal/include"),
                 ]
        ),
        .target(name: "cpe_vfs"
               , path: "vfs"
               , sources: ["src"]
               , cSettings: [
                     .headerSearchPath("../pal/include"),
                     .headerSearchPath("../utils/include"),
                 ]
        ),
        .target(name: "cpe_fsm"
               , path: "fsm"
               , sources: ["src"]
               , cSettings: [
                     .headerSearchPath("../pal/include"),
                     .headerSearchPath("../utils/include"),
                 ]
        ),
        .target(name: "cpe_utils_sock"
               , path: "utils_sock"
               , sources: ["src"]
               , cSettings: [
                     .headerSearchPath("../pal/include"),
                     .headerSearchPath("../utils/include"),
                 ]
        ),
        .target(name: "cpe_utils_json"
               , path: "utils_json"
               , sources: ["src"]
               , cSettings: [
                     .headerSearchPath("../pal/include"),
                     .headerSearchPath("../utils/include"),
                     .headerSearchPath("../depends/yajl/include"),
                 ]
        ),
        .target(name: "log4c"
               , path: "depends/log4c"
               , sources: [
                     "src/rc.c",
                     "src/init.c",
                     "src/appender_type_stream.c",
                     "src/appender_type_stream2.c",
                     "src/layout_type_basic.c",
                     "src/layout_type_dated.c",
                     "src/layout_type_basic_r.c",
                     "src/layout_type_dated_r.c",
                     "src/version.c",
                     "src/logging_event.c",
                     "src/priority.c",
                     "src/appender.c",
                     "src/layout.c",
                     "src/category.c",
                     "src/appender_type_rollingfile.c",
                     "src/rollingpolicy.c",
                     "src/rollingpolicy_type_sizewin.c",
                     "src/sd/stack.c",
                     "src/sd/list.c",
                     "src/sd/malloc.c",
                     "src/sd/factory.c",
                     "src/sd/hash.c",
                     "src/sd/sprintf.c",
                     "src/sd/test.c",
                     "src/sd/sd_xplatform.c",
                     "src/sd/error.c",
                     "src/sd/domnode.c",
                     "src/sd/domnode-xml.c",
                     "src/sd/domnode-xml-parser.c",
                     "src/sd/domnode-xml-scanner.c",
                     "src/appender_type_syslog.c",
                     "src/appender_type_mmap.c",
                 ]
               , cSettings: [
                     .define("HAVE_CONFIG_H"),
                     .define("LOG4C_RCPATH", to: ""),
                     .headerSearchPath("src"),
                     .headerSearchPath("src/ios", .when(platforms: [ .iOS ])),
                     .headerSearchPath("src/mac", .when(platforms: [ .macOS ])),
                 ]
        ),
        .target(name: "yajl"
               , path: "depends/yajl"
               , sources: ["src"]
        ),
        .target(name: "pcre2"
               , path: "depends/pcre2"
               , sources: [
                     "src/pcre2_auto_possess.c",
                     "src/pcre2_chartables.c",
                     "src/pcre2_compile.c",
                     "src/pcre2_config.c",
                     "src/pcre2_context.c",
                     "src/pcre2_dfa_match.c",
                     "src/pcre2_error.c",
                     "src/pcre2_find_bracket.c",
                     "src/pcre2_jit_compile.c",
                     "src/pcre2_maketables.c",
                     "src/pcre2_match.c",
                     "src/pcre2_match_data.c",
                     "src/pcre2_newline.c",
                     "src/pcre2_ord2utf.c",
                     "src/pcre2_pattern_info.c",
                     "src/pcre2_serialize.c",
                     "src/pcre2_string_utils.c",
                     "src/pcre2_study.c",
                     "src/pcre2_substitute.c",
                     "src/pcre2_substring.c",
                     "src/pcre2_tables.c",
                     "src/pcre2_ucd.c",
                     "src/pcre2_valid_utf.c",
                     "src/pcre2_xclass.c",
                 ]
               , cSettings: [
                     .define("PCRE2_STATIC"),
                     .define("HAVE_CONFIG_H"),
                     .define("PCRE2_CODE_UNIT_WIDTH", to: "8"),
                     .headerSearchPath("include"),
                 ]
        ),
    ]
)

