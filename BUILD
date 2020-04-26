load("@//:config.bzl", "platform_config", "package_copt")

platform_config()

cc_library(
    name = "pipe",
    hdrs = [
        "include/pipe.h"
    ],
    strip_include_prefix = "include",
    copts = package_copt,
    visibility = ["//visibility:public"],
)

cc_test(
    name = "test_pipe",
    srcs = ["test/test_pipe.cpp"],
    copts = package_copt,
    tags = ["unit"],
    deps = [
        ":pipe",
        "@gtest//:gtest",
    ],
    visibility = ["//visibility:private"]
)
