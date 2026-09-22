#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
BUILD_DIR=${BUILD_DIR:-build_dir/x64-tests}
mkdir -p "$BUILD_DIR"
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -pedantic -fsanitize=undefined \
    tests/x64_features.c -o "$BUILD_DIR/features"
"$BUILD_DIR/features"
"${CXX:-c++}" -std=c++11 -Icommon -pthread -fsanitize=undefined \
    tests/x64_registration.cpp common/table.cpp common/smtl.cpp \
    -o "$BUILD_DIR/registration"
"$BUILD_DIR/registration"
