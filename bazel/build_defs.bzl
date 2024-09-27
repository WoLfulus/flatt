def vcpkg():
    native.config_setting(
        name = "windows",
        values = {"cpu": "x64_windows"},
    )

    native.config_setting(
        name = "linux",
        values = {"cpu": "x64_linux"},
    )

    native.config_setting(
        name = "arm_linux",
        values = {"cpu": "arm"},
    )

    native.config_setting(
        name = "static",
        values = {"linkstatic": "1"},
    )

    native.config_setting(
        name = "dynamic",
        values = {"linkstatic": "0"},
    )

    native.constraint_setting(name = "arch")

def vcpkg_library(name):
    root = "$(VCPKG_ROOT)/installed/$(VCPKG_DEFAULT_TRIPLET)"

    native.cc_library(
        name = name,
        includes = [
            root + "/include/" + name,
        ],
        linkopts = select({
            "@bazel_tools//src/conditions:windows": select({
                "@bazel_tools//tools/cpp:dbg": [root + "/debug/lib/" + name + ".lib"],
                "@bazel_tools//tools/cpp:opt": [root + "/lib/" + name + ".lib"],
            }),
            "@bazel_tools//src/conditions:linux": select({
                "@bazel_tools//tools/cpp:dbg": [root + "/debug/lib/" + name + ".a"],
                "@bazel_tools//tools/cpp:opt": [root + "/lib/" + name + ".a"],
            }),
        }),
    )
