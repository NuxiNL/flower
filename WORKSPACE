workspace(name = "com_github_nuxinl_flower")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "com_github_nuxinl_argdata",
    commit = "6299455171a28831876d078c59a6634de6f6700b",
    remote = "https://github.com/NuxiNL/argdata.git",
)

git_repository(
    name = "com_github_nuxinl_arpc",
    commit = "58e54234bb0e493b9d3aca5ddc5f1e216083588e",
    remote = "https://github.com/NuxiNL/arpc.git",
)

git_repository(
    name = "io_bazel_rules_python",
    commit = "e6399b601e2f72f74e5aa635993d69166784dde1",
    remote = "https://github.com/bazelbuild/rules_python.git",
)

# TODO(ed): Remove the dependencies below once Bazel supports transitive
# dependency loading. These are from ARPC.
# https://github.com/bazelbuild/proposals/blob/master/designs/2018-11-07-design-recursive-workspaces.md

load("@io_bazel_rules_python//python:pip.bzl", "pip_import", "pip_repositories")

pip_repositories()

pip_import(
    name = "aprotoc_deps",
    requirements = "@com_github_nuxinl_arpc//scripts:requirements.txt",
)

load("@aprotoc_deps//:requirements.bzl", "pip_install")

pip_install()
