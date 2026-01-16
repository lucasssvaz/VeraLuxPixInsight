#!/bin/bash

# This script sets the PCL build environment variables for the VeraLux module.
# It also creates the PCL build directories if they do not exist.
# It should be sourced by the scripts that need to build the module.
#
# Usage:
#   source set_build_env.sh [PCL_PATH]
#
# If PCL_PATH is not provided, defaults to ../PCL relative to repository root
# You can also set PCLDIR environment variable before sourcing this script

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

# Create the PCL build directories
mkdir -p "$PCLDIR/bin/bin64" "$PCLDIR/lib/lib64"

# Set the PCL build environment variables
PCLBINDIR64="$PCLDIR/bin/bin64"
PCLLIBDIR64="$PCLDIR/lib/lib64"
PCLINCDIR="$PCLDIR/include"
PCLSRCDIR="$PCLDIR/src"
PCLBINDIR="$PCLBINDIR64"
PCLLIBDIR="$PCLLIBDIR64"

export PCLDIR
export PCLBINDIR64
export PCLLIBDIR64
export PCLINCDIR
export PCLSRCDIR
export PCLBINDIR
export PCLLIBDIR
