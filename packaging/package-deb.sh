#!/bin/bash
################################################################################
# package-deb.sh - Build GrumpyChat Debian/Ubuntu packages
################################################################################

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/config"

NCPUS=$(nproc)
QT_MAJOR=""
BUILD_JOBS="$NCPUS"

while [[ $# -gt 0 ]]; do
    case $1 in
        --jobs)
            BUILD_JOBS="$2"
            shift 2
            ;;
        --qt5)
            QT_MAJOR=5
            shift
            ;;
        --qt6)
            QT_MAJOR=6
            shift
            ;;
        --version)
            APP_VERSION="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--jobs n] [--qt5|--qt6] [--version x.y.z]"
            exit 1
            ;;
    esac
done

if [ -z "$QT_MAJOR" ]; then
    deb_pkgs_installed() {
        local pkg
        for pkg in "$@"; do
            dpkg-query -W -f='${Status}' "$pkg" 2>/dev/null | grep -q "install ok installed" || return 1
        done
        return 0
    }

    if command -v dpkg-query >/dev/null 2>&1 &&
       deb_pkgs_installed qt6-base-dev qt6-declarative-dev qt6-multimedia-dev; then
        QT_MAJOR=6
    elif command -v dpkg-query >/dev/null 2>&1 &&
         deb_pkgs_installed qtbase5-dev qtdeclarative5-dev qtmultimedia5-dev; then
        QT_MAJOR=5
    else
        QT_MAJOR=6
    fi
fi

echo "================================"
echo "Building GrumpyChat DEB packages"
echo "================================"
echo "CPUs: $NCPUS"
echo "Build jobs: $BUILD_JOBS"
echo "Version: $APP_VERSION"
echo "Qt major: $QT_MAJOR"
echo ""

if command -v dpkg-query >/dev/null 2>&1; then
    missing=()

    require_pkg() {
        local pkg="$1"
        if ! dpkg-query -W -f='${Status}' "$pkg" 2>/dev/null | grep -q "install ok installed"; then
            missing+=("$pkg")
        fi
    }

    require_pkg build-essential
    require_pkg debhelper
    require_pkg dpkg-dev
    require_pkg cmake
    require_pkg pkg-config

    if [ "$QT_MAJOR" -eq 6 ]; then
        require_pkg qt6-base-dev
        require_pkg qt6-declarative-dev
        require_pkg qt6-multimedia-dev
    else
        require_pkg qtbase5-dev
        require_pkg qtdeclarative5-dev
        require_pkg qtmultimedia5-dev
    fi

    if [ ${#missing[@]} -gt 0 ]; then
        echo "Missing build dependencies detected."
        echo ""
        if command -v apt-get >/dev/null 2>&1; then
            echo "  sudo apt-get install ${missing[*]}"
        else
            for pkg in "${missing[@]}"; do
                echo "  - $pkg"
            done
        fi
        echo ""
        exit 1
    fi
fi

PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
DEBIAN_DIR="$PROJECT_ROOT/debian"
CHANGELOG_PATH="$DEBIAN_DIR/changelog"

if [ ! -d "$DEBIAN_DIR" ]; then
    echo "Error: Debian packaging metadata not found at $DEBIAN_DIR"
    exit 1
fi

DISTRO_TAG=""
if [ -r /etc/os-release ]; then
    . /etc/os-release
    if [ "${ID:-}" = "debian" ] && [ -n "${VERSION_ID:-}" ]; then
        DISTRO_TAG="deb${VERSION_ID}"
    elif [ "${ID:-}" = "ubuntu" ] && [ -n "${VERSION_ID:-}" ]; then
        DISTRO_TAG="ubuntu${VERSION_ID}"
    fi
fi

if [ -n "$DISTRO_TAG" ]; then
    DEB_VERSION="${APP_VERSION}~${DISTRO_TAG}"
else
    DEB_VERSION="${APP_VERSION}"
fi

BACKUP_CHANGELOG=""
if [ -f "$CHANGELOG_PATH" ]; then
    BACKUP_CHANGELOG=$(mktemp)
    cp "$CHANGELOG_PATH" "$BACKUP_CHANGELOG"
fi

trap 'if [ -n "$BACKUP_CHANGELOG" ] && [ -f "$BACKUP_CHANGELOG" ]; then cp "$BACKUP_CHANGELOG" "$CHANGELOG_PATH"; rm -f "$BACKUP_CHANGELOG"; fi' EXIT

cat > "$CHANGELOG_PATH" <<EOF
${APP_NAME} (${DEB_VERSION}) unstable; urgency=medium

  * Automated build.

 -- ${MAINTAINER}  $(date -R)
EOF

cd "$PROJECT_ROOT"
export GRUMPY_QT_MAJOR="$QT_MAJOR"
export GRUMPY_BUILD_JOBS="$BUILD_JOBS"
dpkg-buildpackage -b -us -uc

OUTPUT_DIR="$PROJECT_ROOT/packaging/output"
OUTPUT_PARENT="$(cd "$PROJECT_ROOT/.." && pwd)"
mkdir -p "$OUTPUT_DIR"

shopt -s nullglob
DEB_FILES=("$OUTPUT_PARENT"/grumpy*_"$DEB_VERSION"_*.deb "$PROJECT_ROOT"/grumpy*_"$DEB_VERSION"_*.deb)
shopt -u nullglob

if [ ${#DEB_FILES[@]} -eq 0 ]; then
    echo "Error: No .deb artifacts found."
    find "$OUTPUT_PARENT" "$PROJECT_ROOT" -maxdepth 1 -type f -name "*.deb" -print
    exit 1
fi

for DEB_FILE in "${DEB_FILES[@]}"; do
    mv "$DEB_FILE" "$OUTPUT_DIR/"
done

echo ""
echo "Build complete. Packages:"
find "$OUTPUT_DIR" -maxdepth 1 -type f -name "grumpy*_${DEB_VERSION}_*.deb" -print | sort
