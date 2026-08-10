#!/bin/bash

# This script sets the PCL build environment variables for the VeraLux module.
# Paths follow the PCL README conventions:
#   https://gitlab.com/pixinsight/PCL/-/blob/master/README.md
#
# Usage:
#   source set_build_env.sh [PCL_PATH]
#
# Optional inputs (from caller / environment):
#   PLATFORM      linux|macosx|windows
#   MACOSX_ARCH   x64|arm64|all  (macosx only; "all" uses host arch for paths)
#
# If PCL_PATH is not provided, defaults to ../PCL relative to repository root.
# You can also set PCLDIR environment variable before sourcing this script.

SCRIPTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPTS_DIR/../.." && pwd)"

# Determine PCL directory
# Priority: 1. $1 (argument), 2. $PCLDIR (env var), 3. $REPO_ROOT/../PCL (default)
if [ -n "$1" ]; then
    PCLDIR="$(cd "$1" 2>/dev/null && pwd || echo "$1")"
elif [ -z "$PCLDIR" ]; then
    PCLDIR="$REPO_ROOT/../PCL"
fi

# Resolve to absolute path
if [ -d "$PCLDIR" ]; then
    PCLDIR="$(cd "$PCLDIR" && pwd)"
else
    echo "Warning: PCL directory not found at: $PCLDIR"
    echo "PCL will need to be cloned or path needs to be corrected"
fi

# Resolve platform / arch for library layout: $PCLDIR/lib/<platform>/<arch>
_pcl_platform="${PLATFORM:-}"
_pcl_arch="x64"

if [ -z "$_pcl_platform" ]; then
    case "$(uname -s)" in
        Linux*)   _pcl_platform="linux" ;;
        Darwin*)  _pcl_platform="macosx" ;;
        MINGW*|MSYS*|CYGWIN*) _pcl_platform="windows" ;;
        *)        _pcl_platform="linux" ;;
    esac
fi

case "$_pcl_platform" in
    macosx)
        if [ -n "$MACOSX_ARCH" ] && [ "$MACOSX_ARCH" != "all" ]; then
            _pcl_arch="$MACOSX_ARCH"
        elif [ "$(uname -m)" = "arm64" ]; then
            _pcl_arch="arm64"
        else
            _pcl_arch="x64"
        fi
        ;;
    linux|windows)
        _pcl_arch="x64"
        ;;
esac

# PCL README:
#   PCLBINDIR64 = $PCLDIR/bin
#   PCLLIBDIR64 = $PCLDIR/lib/[platform]/x64  (arm64 uses lib/macosx/arm64)
PCLBINDIR64="$PCLDIR/bin"
PCLLIBDIR64="$PCLDIR/lib/${_pcl_platform}/${_pcl_arch}"
PCLINCDIR="$PCLDIR/include"
PCLSRCDIR="$PCLDIR/src"
PCLBINDIR="$PCLBINDIR64"
PCLLIBDIR="$PCLLIBDIR64"

mkdir -p "$PCLBINDIR64" "$PCLLIBDIR64"

export PCLDIR
export PCLBINDIR64
export PCLLIBDIR64
export PCLINCDIR
export PCLSRCDIR
export PCLBINDIR
export PCLLIBDIR
