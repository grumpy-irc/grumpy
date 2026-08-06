#!/bin/bash
################################################################################
# package-rpm.sh - Build GrumpyChat RPM packages
################################################################################

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/config"

NCPUS=$(nproc)
RELEASE="1"
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
    if command -v rpm >/dev/null 2>&1 &&
       rpm -q qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtmultimedia-devel >/dev/null 2>&1; then
        QT_MAJOR=6
    elif command -v rpm >/dev/null 2>&1 &&
         rpm -q qt5-qtbase-devel qt5-qtdeclarative-devel qt5-qtmultimedia-devel >/dev/null 2>&1; then
        QT_MAJOR=5
    else
        QT_MAJOR=6
    fi
fi

echo "================================"
echo "Building GrumpyChat RPM packages"
echo "================================"
echo "CPUs: $NCPUS"
echo "Build jobs: $BUILD_JOBS"
echo "Version: $APP_VERSION-$RELEASE"
echo "Qt major: $QT_MAJOR"
echo ""

if command -v rpm >/dev/null 2>&1; then
    missing=()

    require_pkg() {
        local pkg="$1"
        if ! rpm -q "$pkg" >/dev/null 2>&1; then
            missing+=("$pkg")
        fi
    }

    require_pkg rpm-build
    require_pkg cmake
    require_pkg gcc-c++
    require_pkg make
    require_pkg pkgconf-pkg-config

    if [ "$QT_MAJOR" -eq 6 ]; then
        require_pkg qt6-qtbase-devel
        require_pkg qt6-qtdeclarative-devel
        require_pkg qt6-qtmultimedia-devel
    else
        require_pkg qt5-qtbase-devel
        require_pkg qt5-qtdeclarative-devel
        require_pkg qt5-qtmultimedia-devel
    fi

    if [ ${#missing[@]} -gt 0 ]; then
        echo "Missing build dependencies detected."
        echo ""
        if command -v dnf >/dev/null 2>&1; then
            echo "  sudo dnf install ${missing[*]}"
        elif command -v yum >/dev/null 2>&1; then
            echo "  sudo yum install ${missing[*]}"
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
RPM_TOPDIR="$HOME/rpmbuild"
mkdir -p "$RPM_TOPDIR"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

TEMP_DIR=$(mktemp -d -p "$RPM_TOPDIR" grumpy-rpm-XXXXXX)
trap "rm -rf $TEMP_DIR" EXIT

SOURCE_DIR_NAME="${APP_NAME}-${APP_VERSION}"
SOURCE_TARBALL="$RPM_TOPDIR/SOURCES/$SOURCE_DIR_NAME.tar.gz"
tar -C "$PROJECT_ROOT" \
    --exclude='*/.git*' \
    --exclude='*/build-ai-packaging' \
    --exclude='packaging/output' \
    --transform "s#^#$SOURCE_DIR_NAME/#" \
    -czf "$SOURCE_TARBALL" \
    README LICENSE.txt configure doc linux snap src tools travis windows windows64 test_cert packaging debian

SPEC_PATH="$RPM_TOPDIR/SPECS/$APP_NAME.spec"
cat > "$SPEC_PATH" <<EOF_SPEC
%global debug_package %{nil}

Name:           $APP_NAME
Version:        $APP_VERSION
Release:        $RELEASE%{?dist}
Summary:        $DESCRIPTION

License:        GPL-3.0-or-later
URL:            $APP_HOMEPAGE_URL
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  pkgconf-pkg-config
%if $QT_MAJOR == 6
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qtmultimedia-devel
%else
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qtmultimedia-devel
%endif

Requires:       %{name}-common%{?_isa} = %{version}-%{release}
Requires:       hicolor-icon-theme

%description
GrumpyChat is a Qt IRC client with a distributed core, optional daemon, and
JavaScript scripting support.

%package common
Summary:        Shared private libraries for GrumpyChat

%description common
This package contains private shared libraries used by GrumpyChat binaries.

%package -n grumpyd
Summary:        GrumpyChat distributed-core daemon
Requires:       %{name}-common%{?_isa} = %{version}-%{release}

%description -n grumpyd
grumpyd is the optional distributed-core daemon for GrumpyChat.

%prep
%autosetup

%build
cmake -S src -B build \\
  -DCMAKE_BUILD_TYPE=Release \\
  -DCMAKE_INSTALL_PREFIX=%{_prefix} \\
  -DCMAKE_INSTALL_LIBDIR=%{_lib} \\
  -DQT5_BUILD=$(if [ "$QT_MAJOR" -eq 6 ]; then echo false; else echo true; fi) \\
  -DQT6_BUILD=$(if [ "$QT_MAJOR" -eq 6 ]; then echo true; else echo false; fi) \\
  -Wno-dev
cmake --build build --parallel $BUILD_JOBS

%install
rm -rf %{buildroot}
DESTDIR=%{buildroot} cmake --install build
install -Dm644 packaging/data/grumpy.desktop %{buildroot}%{_datadir}/applications/grumpy.desktop
install -Dm644 snap/gui/icon.png %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/grumpy.png
install -Dm644 README %{buildroot}%{_docdir}/%{name}/README
install -Dm644 LICENSE.txt %{buildroot}%{_docdir}/%{name}/LICENSE.txt

%files
%doc %{_docdir}/%{name}/README
%license %{_docdir}/%{name}/LICENSE.txt
%{_bindir}/grumpy
%{_libdir}/libgrumpy.so*
%{_datadir}/applications/grumpy.desktop
%{_datadir}/icons/hicolor/256x256/apps/grumpy.png

%files common
%{_libdir}/libgp.so*
%{_libdir}/libgrumpycore.so*
%{_libdir}/libirc.so*
%{_libdir}/libirc2htmlcode.so*
%{_libdir}/libircclient.so*

%post common -p /sbin/ldconfig
%postun common -p /sbin/ldconfig

%files -n grumpyd
%{_bindir}/grumpyd

%changelog
* $(date '+%a %b %d %Y') $MAINTAINER - $APP_VERSION-$RELEASE
- Automated package build
EOF_SPEC

rpmbuild --define "_topdir $RPM_TOPDIR" -ba "$SPEC_PATH"

OUTPUT_DIR="$PROJECT_ROOT/packaging/output"
mkdir -p "$OUTPUT_DIR"
find "$RPM_TOPDIR/RPMS" "$RPM_TOPDIR/SRPMS" -type f \( -name "grumpy-${APP_VERSION}-${RELEASE}*.rpm" -o -name "grumpy-common-${APP_VERSION}-${RELEASE}*.rpm" -o -name "grumpyd-${APP_VERSION}-${RELEASE}*.rpm" \) -exec cp {} "$OUTPUT_DIR/" \;

echo ""
echo "Build complete. Packages:"
find "$OUTPUT_DIR" -maxdepth 1 -type f \( -name "grumpy-${APP_VERSION}-${RELEASE}*.rpm" -o -name "grumpy-common-${APP_VERSION}-${RELEASE}*.rpm" -o -name "grumpyd-${APP_VERSION}-${RELEASE}*.rpm" \) -print | sort
