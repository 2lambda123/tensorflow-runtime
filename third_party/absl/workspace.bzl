"""Provides the repository macro to import absl."""

load("//third_party:repo.bzl", "tfrt_http_archive")

def repo(name):
    """Imports absl."""

    # Attention: tools parse and update these lines.
    ABSL_COMMIT = "b971ac5250ea8de900eae9f95e06548d14cd95fe"
    ABSL_SHA256 = "8eeec9382fc0338ef5c60053f3a4b0e0708361375fe51c9e65d0ce46ccfe55a7"

    SYS_DIRS = [
        "algorithm",
        "base",
        "cleanup",
        "container",
        "debugging",
        "flags",
        "functional",
        "hash",
        "memory",
        "meta",
        "numeric",
        "random",
        "status",
        "strings",
        "synchronization",
        "time",
        "types",
        "utility",
    ]
    SYS_LINKS = {
        "//third_party/absl:system.absl.{name}.BUILD".format(name = n): "absl/{name}/BUILD.bazel".format(name = n)
        for n in SYS_DIRS
    }

    tfrt_http_archive(
        name = name,
        sha256 = ABSL_SHA256,
        build_file = "//third_party/absl:com_google_absl.BUILD",
        system_build_file = "//third_party/absl:system.BUILD",
        system_link_files = SYS_LINKS,
        # This patch pulls in a fix for designated initializers that MSVC
        # complains about. It shouldn't be necessary at the next LTS release.
        patch_file = "//third_party/absl:absl_designated_initializers.patch",
        strip_prefix = "abseil-cpp-{commit}".format(commit = ABSL_COMMIT),
        urls = [
            "https://storage.googleapis.com/mirror.tensorflow.org/github.com/abseil/abseil-cpp/archive/{commit}.tar.gz".format(commit = ABSL_COMMIT),
            "https://github.com/abseil/abseil-cpp/archive/{commit}.tar.gz".format(commit = ABSL_COMMIT),
        ],
    )
