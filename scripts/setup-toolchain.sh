#!/usr/bin/env bash

set -euo pipefail

if ! command -v dnf >/dev/null 2>&1; then
    echo "error: this script expects Fedora (dnf). Install the packages below manually." >&2
    exit 1
fi

PACKAGES=(
    gcc make binutils bear clang-tools-extra gdb valgrind
    strace ltrace libasan libubsan jemalloc-devel mimalloc-devel
)

echo "Installing: ${PACKAGES[*]}"
sudo dnf install -y "${PACKAGES[@]}"

echo
echo "Toolchain ready:"
gcc --version | head -n1
make --version | head -n1
