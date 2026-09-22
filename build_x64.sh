#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

BUILD_DIR=${BUILD_DIR:-build_dir/x64}
OUTPUT=${OUTPUT:-cpufp}
CC=${CC:-cc}
CXX=${CXX:-c++}
arch_flags=(-march=x86-64 -mtune=generic)
link_flags=(-pthread)
os=$(uname -s)
case "$os" in
    Darwin)
        arch_flags+=(-arch x86_64 "-mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET:-11.0}")
        ;;
    Linux)
        link_flags+=(-Wl,-z,noexecstack)
        ;;
    *) echo 'Error: x64 builds support Linux and macOS.' >&2; exit 1 ;;
esac
mkdir -p "$BUILD_DIR"

"$CXX" "${arch_flags[@]}" -std=c++11 -O3 -c common/table.cpp -o "$BUILD_DIR/table.o"
"$CXX" "${arch_flags[@]}" -std=c++11 -O3 -pthread -c common/smtl.cpp -o "$BUILD_DIR/smtl.o"
"$CC" "${arch_flags[@]}" -O2 x64/cpuid.c -o "$BUILD_DIR/cpuid"
"$CC" "${arch_flags[@]}" -O2 -DCPUFP_CPUID_NO_MAIN -c x64/cpuid.c -o "$BUILD_DIR/features.o"

# Compile every kernel the assembler supports. Execution is gated at runtime,
# allowing a binary to move to another CPU without using build-host features.
simd_macros=()
simd_objects=()
for source in x64/asm/*.S; do
    simd=${source##*/}
    simd=${simd%.S}
    if [[ $os == Darwin && $simd == _AMX_* ]]; then
        continue
    fi
    if "$CC" "${arch_flags[@]}" -c "$source" -o "$BUILD_DIR/$simd.o" 2>"$BUILD_DIR/$simd.log"; then
        simd_macros+=("-D$simd")
        simd_objects+=("$BUILD_DIR/$simd.o")
    else
        cat "$BUILD_DIR/$simd.log" >&2
        if [[ $simd == _SSE_ || $simd == _SSE2_ ]]; then
            exit 1
        fi
        echo "Warning: assembler cannot build $simd; skipping this ISA." >&2
    fi
done

"$CXX" "${arch_flags[@]}" -std=c++11 -O3 -Icommon "${simd_macros[@]}" \
    -c x64/cpufp.cpp -o "$BUILD_DIR/cpufp.o"
"$CXX" "${arch_flags[@]}" "${link_flags[@]}" -o "$OUTPUT" \
    "$BUILD_DIR/cpufp.o" "$BUILD_DIR/features.o" "$BUILD_DIR/smtl.o" \
    "$BUILD_DIR/table.o" "${simd_objects[@]}"
echo "Built $OUTPUT (x86-64)."
