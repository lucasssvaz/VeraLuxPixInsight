#!/bin/bash
#
# PixInsight Module Signing Script
# Signs a PixInsight module binary with an XSSK key
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Variables
MODULE_FILE=""
XSSK_FILE=""
XSSK_PASSWORD=""

# Parse command line arguments
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
            echo "Usage: $0 --module-file=<path> --xssk-file=<path> --xssk-password=<password>"
            echo ""
            echo "Signs a PixInsight module binary with an XSSK key."
            echo ""
            echo "Options:"
            echo "  --module-file=<path>      Path to the module binary to sign"
            echo "  --xssk-file=<path>        Path to the XSSK key file"
            echo "  --xssk-password=<password> Password for the XSSK key"
            echo "  -h, --help                Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0 --module-file=bin/macos/VeraLuxPixInsight-pxm.dylib \\"
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

# Validate required arguments
if [ -z "$MODULE_FILE" ]; then
    log_error "Missing required argument: --module-file"
    echo "Use --help for usage information"
    exit 1
fi

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

# Validate files exist
if [ ! -f "$MODULE_FILE" ]; then
    log_error "Module file not found: $MODULE_FILE"
    exit 1
fi

if [ ! -f "$XSSK_FILE" ]; then
    log_error "XSSK key file not found: $XSSK_FILE"
    exit 1
fi

# Check if PixInsight is available
if ! command -v PixInsight &> /dev/null; then
    log_error "PixInsight executable not found in PATH"
    log_info "Please ensure PixInsight is installed and added to your PATH"
    exit 1
fi

# Sign the module
log_info "Signing module: $MODULE_FILE"
log_info "Using key: $XSSK_FILE"

PixInsight --sign-module-file="$MODULE_FILE" --xssk-file="$XSSK_FILE" --xssk-password="$XSSK_PASSWORD"

if [ $? -eq 0 ]; then
    log_success "Module signed successfully!"
    log_info "Signed binary: $MODULE_FILE"
else
    log_error "Module signing failed"
    exit 1
fi
