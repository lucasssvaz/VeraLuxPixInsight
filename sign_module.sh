#!/bin/bash
#
# PixInsight Module Signing Script
# Signs one or all VeraLuxPixInsight module binaries with an XSSK key
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"

MODULE_FILE=""
XSSK_FILE=""
XSSK_PASSWORD=""

# Default candidates when --module-file is omitted
DEFAULT_MODULES=(
    "bin/linux/VeraLuxPixInsight-pxm.so"
    "bin/macosx/x64/VeraLuxPixInsight-pxm.dylib"
    "bin/macosx/arm64/VeraLuxPixInsight-pxm.dylib"
    "bin/windows/VeraLuxPixInsight-pxm.dll"
)

while [[ $# -gt 0 ]]; do
    case $1 in
        --module-file=*)
            MODULE_FILE="${1#*=}"
            shift
            ;;
        --xssk-file=*)
            XSSK_FILE="${1#*=}"
            shift
            ;;
        --xssk-password=*)
            XSSK_PASSWORD="${1#*=}"
            shift
            ;;
        -h|--help)
            echo "Usage: $0 --xssk-file=<path> --xssk-password=<password> [--module-file=<path>]"
            echo ""
            echo "Signs VeraLuxPixInsight module binaries with an XSSK key."
            echo "If --module-file is omitted, every built binary under bin/ is signed."
            echo ""
            echo "Options:"
            echo "  --xssk-file=<path>         Path to the XSSK key file"
            echo "  --xssk-password=<password> Password for the XSSK key"
            echo "  --module-file=<path>       Optional path to a single module binary"
            echo "  -h, --help                 Show this help message"
            echo ""
            echo "Examples:"
            echo "  # Sign all built platforms/arches"
            echo "  $0 --xssk-file=/path/to/key.xssk --xssk-password=mypassword"
            echo ""
            echo "  # Sign one binary"
            echo "  $0 --module-file=bin/macosx/arm64/VeraLuxPixInsight-pxm.dylib \\"
            echo "     --xssk-file=/path/to/key.xssk \\"
            echo "     --xssk-password=mypassword"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

if [ -z "$XSSK_FILE" ]; then
    log_error "Missing required argument: --xssk-file"
    echo "Use --help for usage information"
    exit 1
fi

if [ -z "$XSSK_PASSWORD" ]; then
    log_error "Missing required argument: --xssk-password"
    echo "Use --help for usage information"
    exit 1
fi

if [ ! -f "$XSSK_FILE" ]; then
    log_error "XSSK key file not found: $XSSK_FILE"
    exit 1
fi

if ! command -v PixInsight &> /dev/null; then
    log_error "PixInsight executable not found in PATH"
    log_info "Please ensure PixInsight is installed and added to your PATH"
    exit 1
fi

collect_modules() {
    local modules=()

    if [ -n "$MODULE_FILE" ]; then
        if [ ! -f "$MODULE_FILE" ]; then
            log_error "Module file not found: $MODULE_FILE"
            exit 1
        fi
        modules+=("$MODULE_FILE")
    else
        local candidate
        for candidate in "${DEFAULT_MODULES[@]}"; do
            if [ -f "$REPO_ROOT/$candidate" ]; then
                modules+=("$REPO_ROOT/$candidate")
            fi
        done

        # Also pick up any unexpected VeraLux module binaries under bin/
        while IFS= read -r -d '' found; do
            local already=0
            local existing
            for existing in "${modules[@]+"${modules[@]}"}"; do
                if [ "$existing" = "$found" ]; then
                    already=1
                    break
                fi
            done
            if [ "$already" -eq 0 ]; then
                modules+=("$found")
            fi
        done < <(find "$REPO_ROOT/bin" \
            \( -name 'VeraLuxPixInsight-pxm.so' \
               -o -name 'VeraLuxPixInsight-pxm.dylib' \
               -o -name 'VeraLuxPixInsight-pxm.dll' \) \
            -type f -print0 2>/dev/null)
    fi

    if [ "${#modules[@]}" -eq 0 ]; then
        log_error "No module binaries found to sign"
        log_info "Expected one of:"
        local path
        for path in "${DEFAULT_MODULES[@]}"; do
            echo "  - $path"
        done
        exit 1
    fi

    printf '%s\n' "${modules[@]}"
}

sign_one() {
    local module_file="$1"

    log_info "Signing module: $module_file"
    PixInsight \
        --sign-module-file="$module_file" \
        --xssk-file="$XSSK_FILE" \
        --xssk-password="$XSSK_PASSWORD"
    log_success "Signed: $module_file"
}

MODULES=()
while IFS= read -r line; do
    [ -n "$line" ] && MODULES+=("$line")
done < <(collect_modules)

log_info "Using key: $XSSK_FILE"
log_info "Signing ${#MODULES[@]} module binary(ies)"

for module in "${MODULES[@]}"; do
    sign_one "$module"
done

log_success "All module binaries signed successfully"
