# GrumpyChat Packaging

This directory contains local package build scripts for GrumpyChat.

## Supported Targets

- Debian/Ubuntu (`.deb`) via `package-deb.sh`
- Fedora/RHEL family (`.rpm` and `.src.rpm`) via `package-rpm.sh`

Both formats split the install into:

- `grumpy-common`: private shared libraries used by both binaries
- `grumpy`: desktop IRC client
- `grumpyd`: distributed-core daemon

## Debian/Ubuntu Dependencies

```bash
sudo apt-get install build-essential debhelper dpkg-dev cmake pkg-config qt6-base-dev qt6-declarative-dev qt6-multimedia-dev
```

The scripts prefer Qt6 when available. Pass `--qt5` to force a Qt5 build.

## Fedora/RHEL Dependencies

```bash
sudo dnf install gcc-c++ make cmake rpm-build pkgconf-pkg-config qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtmultimedia-devel
```

The scripts prefer Qt6 when available. Pass `--qt5` to force a Qt5 build.

## Usage

From the repository root:

```bash
packaging/package-deb.sh
packaging/package-rpm.sh
```

Optional:

```bash
packaging/package-deb.sh --version 1.2.3
packaging/package-deb.sh --jobs 2
packaging/package-deb.sh --qt5
packaging/package-rpm.sh --version 1.2.3
packaging/package-rpm.sh --jobs 2
packaging/package-rpm.sh --qt5
```

Output is written to `packaging/output/`.
