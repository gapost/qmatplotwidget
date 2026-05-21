#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <version>"
    exit 1
fi

VERSION="$1"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

echo "==> Creating source tarball qmatplotwidget-${VERSION}.tar.gz"
git -C "${REPO_ROOT}" archive \
    --prefix="qmatplotwidget-${VERSION}/" \
    --format=tar.gz \
    HEAD \
    -o "${BUILD_DIR}/qmatplotwidget-${VERSION}.tar.gz"

echo "==> Setting pkgver=${VERSION} in PKGBUILD"
sed "s/^pkgver=.*/pkgver=${VERSION}/" "${SCRIPT_DIR}/PKGBUILD" > "${BUILD_DIR}/PKGBUILD"

echo "==> Building MSYS2/UCRT64 package"
cd "${BUILD_DIR}"
MINGW_ARCH=ucrt64 makepkg-mingw --noconfirm --noprogressbar --skipchecksums

echo "==> Done: $(ls *.pkg.tar.zst)"
