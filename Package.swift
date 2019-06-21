// swift-tools-version:5.0

import PackageDescription

let package = Package(
    name: "cpe",
    products: [
        .library(name: "log4c", type: .static, targets: ["log4c"]),
        .library(name: "cpe_pal", type: .static, targets: ["cpe_pal"]),
        .library(name: "cpe_utils", type: .static, targets: ["cpe_utils"]),
        .library(name: "cpe_utils_sock", type: .static, targets: ["cpe_utils_sock"]),
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
    ]
)

