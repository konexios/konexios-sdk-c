import qbs 1.0
StaticLibrary {
    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [
            ".",
            "include",
            "src/wolfSSL",
            "src/wolfSSL/wolfssl"
        ]

    }

    Depends { name: "cpp" }
    cpp.includePaths: [
        ".",
        "include",
        "src/wolfSSL",
        "src/wolfSSL/wolfssl"
    ]

    name: "kronos-c-sdk"

    files: ["include/config.h", "include/debug.h", "include/unint.h"]

    Group {
        name: "time"
//        fileTags: "cpp"
        files: [
            "include/time/*.h",
            "src/time/*.c"
        ]
    }

    Group {
        name: "ntp"
        fileTags: "cpp"
        files: [
            "include/ntp/*.h",
            "src/ntp/*.c"
        ]
    }

    Group {
        name: "http"
//        fileTags: "cpp"
        files: [
            "include/http/*.h",
            "src/http/*.c"
        ]
    }

    Group {
        name: "bsd"
//        fileTags: "cpp"
        files: [
            "include/bsd/*.h",
            "src/bsd/*.c"
        ]
    }

    Group {
        name: "mqtt"
//        fileTags: "cpp"
        files: [
            "include/mqtt/client/*.h",
            "include/mqtt/packet/*.h",
            "src/mqtt/client/src/*.c",
            "src/mqtt/packet/src/*.c",
            "src/mqtt/client/src/mbed/*.c"
        ]
    }

    Group {
        name: "arrow"
//        fileTags: "cpp"
        files: [
            "src/arrow/*.c",
            "include/arrow/*.h",
        ]
    }

    Group {
        name: "json"
//        fileTags: "cpp"

        files: [
            "src/json/*.c",
            "include/json/*.h"
        ]
    }

    Group {
        name: "crypt"
        files: [
//            "platforms/xtensa/crypt/*.c",
            "platforms/common/wolf/crypt/*.c",
            "include/crypt/*.c"
        ]
    }

    Group {
        name: "ssl"
        prefix: "src/wolfSSL/"
        files: [
            "*.h",
            "src/*.c",
            "wolfcrypt/src/*.c",
            "wolfssl/*.h",
            "wolfssl/wolfcrypt/*.h"
        ]
    }

    Group {
        fileTagsFilter: product.type
        qbs.install: true
        qbs.installDir: "/usr/lib"
    }
}
