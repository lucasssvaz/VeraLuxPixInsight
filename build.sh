#!/bin/bash
#
# VeraLuxPixInsight Build Script
# Builds PCL and the module for Linux, macOS (macosx), or Windows
#

set -e  # Exit on error

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

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Determine script location
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"

# Default values
PLATFORM=""
PCL_PATH=""
MSBUILD_CMD=""  # Global variable to store MSBuild command path

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --platform=*)
            PLATFORM="${1#*=}"
            shift
            ;;
        --pcl-path=*)
            PCL_PATH="${1#*=}"
            shift
            ;;
        *)
            log_error "Unknown option: $1"
            echo "Usage: $0 [--platform=<linux|macosx|windows>] [--pcl-path=<path>]"
            exit 1
            ;;
    esac
done

# Auto-detect platform if not specified
if [ -z "$PLATFORM" ]; then
    case "$(uname -s)" in
        Linux*)
            PLATFORM="linux"
            ;;
        Darwin*)
            PLATFORM="macosx"
            ;;
        MINGW*|MSYS*|CYGWIN*)
            PLATFORM="windows"
            ;;
        *)
            log_error "Unsupported platform: $(uname -s)"
            exit 1
            ;;
    esac
    log_info "Auto-detected platform: $PLATFORM"
fi

# Determine PCL path
if [ -z "$PCL_PATH" ]; then
    PCL_PATH="$REPO_ROOT/../PCL"
fi
PCL_PATH="$(cd "$PCL_PATH" 2>/dev/null && pwd || echo "$PCL_PATH")"

log_info "Repository root: $REPO_ROOT"
log_info "PCL path: $PCL_PATH"
log_info "Target platform: $PLATFORM"

# Function to check dependencies
check_dependencies() {
    log_info "Checking dependencies for $PLATFORM..."

    case "$PLATFORM" in
        linux)
            # PCL README: Ubuntu 22.04 LTS with GCC 12+, C++20
            missing_tools=""
            if ! command -v g++ &> /dev/null; then
                missing_tools="$missing_tools g++"
            fi
            if ! command -v make &> /dev/null; then
                missing_tools="$missing_tools make"
            fi
            if ! command -v python3 &> /dev/null; then
                missing_tools="$missing_tools python3"
            fi

            if [ -n "$missing_tools" ]; then
                log_error "Missing required tools:$missing_tools"
                log_error "PCL requires GCC 12+ (see https://gitlab.com/pixinsight/PCL)"
                exit 1
            fi

            GCC_VERSION=$(g++ -dumpversion 2>/dev/null | cut -d. -f1)
            if [ -z "$GCC_VERSION" ] || [ "$GCC_VERSION" -lt 12 ]; then
                log_error "GCC 12 or later required, found: $(g++ --version | head -1)"
                log_error "PCL README: Ubuntu 22.04 LTS with GCC 12.3.0"
                exit 1
            fi
            log_success "GCC $(g++ -dumpversion) found (C++20 required)"
            ;;
        macosx)
            # PCL README: macOS 15+ with Clang 17+ / Xcode 26+
            if ! command -v clang++ &> /dev/null; then
                log_error "Clang not found"
                log_error "Please install Xcode 26+ from the Mac App Store or developer.apple.com"
                exit 1
            fi

            if ! command -v xcodebuild &> /dev/null; then
                log_error "xcodebuild not found"
                log_error "Please ensure Xcode 26+ is properly installed"
                exit 1
            fi

            XCODE_VERSION=$(xcodebuild -version 2>/dev/null | head -1 | sed 's/Xcode //' | cut -d'.' -f1)
            if [ -z "$XCODE_VERSION" ] || [ "$XCODE_VERSION" -lt 26 ]; then
                log_error "Xcode 26 or later required, found: $(xcodebuild -version 2>/dev/null | head -1)"
                log_error "PCL README: Clang 17+ with Xcode 26 (macOS 15+)"
                exit 1
            fi
            log_success "Xcode $(xcodebuild -version | head -1 | sed 's/Xcode //') found"
            log_success "Clang $(clang++ --version | head -1)"
            ;;
        windows)
            log_info "Checking for Visual Studio 2022 (17.x) with C++ tools..."
            
            # Check for msbuild in PATH (try both with and without .exe extension)
            if command -v msbuild.exe &> /dev/null; then
                # Get full path to msbuild.exe
                MSBUILD_CMD=$(command -v msbuild.exe)
            elif command -v msbuild &> /dev/null; then
                # Get full path to msbuild
                MSBUILD_CMD=$(command -v msbuild)
            else
                # Try to find MSBuild using vswhere (if available)
                # Specifically look for Visual Studio 2022 (version 17.x)
                VSWHERE="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
                if [ -f "$VSWHERE" ]; then
                    MSBUILD_PATH=$("$VSWHERE" -version "[17.0,18.0)" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe 2>/dev/null | head -1)
                    if [ -n "$MSBUILD_PATH" ] && [ -f "$MSBUILD_PATH" ]; then
                        MSBUILD_DIR=$(dirname "$MSBUILD_PATH")
                        export PATH="$MSBUILD_DIR:$PATH"
                        MSBUILD_CMD="$MSBUILD_PATH"
                        log_success "msbuild found via vswhere at: $MSBUILD_PATH"
                        log_info "Added MSBuild directory to PATH: $MSBUILD_DIR"
                    else
                        log_error "msbuild 17.x not found in PATH or via vswhere"
                        log_error "Please ensure Visual Studio 2022 (17.x) is installed with C++ tools"
                        exit 1
                    fi
                else
                    log_error "msbuild not found in PATH and vswhere.exe not available"
                    log_error "Please ensure Visual Studio 2022 (17.x) is installed with C++ tools"
                    exit 1
                fi
            fi
            
            # Verify MSBuild version is 17.x
            if [ -n "$MSBUILD_CMD" ]; then
                log_info "Verifying MSBuild version..."
                VERSION_OUTPUT=$("$MSBUILD_CMD" -version 2>&1 | head -1)
                # Extract version pattern from formats like:
                # "MSBuild version 17.14.23+b0019275e for .NET Framework"
                # "17.14.23+b0019275e"
                # Extract the first X.Y.Z pattern found
                FULL_VERSION=$(echo "$VERSION_OUTPUT" | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
                if [ -n "$FULL_VERSION" ]; then
                    # Extract major version (first number)
                    MAJOR_VERSION=$(echo "$FULL_VERSION" | cut -d. -f1)
                    if [ "$MAJOR_VERSION" = "17" ]; then
                        log_success "msbuild version verified: $FULL_VERSION (17.x)"
                    else
                        log_error "MSBuild version $FULL_VERSION found, but version 17.x is required"
                        log_error "Found version output: $VERSION_OUTPUT"
                        log_error "Please install Visual Studio 2022 (17.x) with C++ tools"
                        exit 1
                    fi
                else
                    log_warning "Could not parse MSBuild version from: $VERSION_OUTPUT"
                    log_warning "Continuing anyway, but version 17.x is required"
                fi
            fi
            ;;
    esac
}

# Function to clone PCL if needed
clone_pcl() {
    if [ ! -d "$PCL_PATH" ]; then
        log_info "PCL not found at $PCL_PATH"
        log_info "Cloning PCL from GitLab..."
        
        PCL_PARENT="$(dirname "$PCL_PATH")"
        mkdir -p "$PCL_PARENT"
        cd "$PCL_PARENT"
        
        git clone https://gitlab.com/pixinsight/PCL.git
        
        if [ ! -d "$PCL_PATH" ]; then
            log_error "Failed to clone PCL"
            exit 1
        fi
        
        log_success "PCL cloned successfully"
    else
        log_success "PCL found at $PCL_PATH"
    fi
}

# Function to source PCL build environment
source_pcl_env() {
    log_info "Setting up PCL build environment..."
    
    SET_BUILD_ENV="$REPO_ROOT/.github/scripts/set_build_env.sh"
    
    if [ ! -f "$SET_BUILD_ENV" ]; then
        log_error "set_build_env.sh not found at $SET_BUILD_ENV"
        exit 1
    fi
    
    
    # Source the environment script
    source "$SET_BUILD_ENV"
    
    log_success "PCL environment configured"
    log_info "PCLDIR=$PCLDIR"
    log_info "PCLBINDIR64=$PCLBINDIR64"
    log_info "PCLLIBDIR64=$PCLLIBDIR64"
}

# Resolve macOS build architecture: x64, arm64, or all
# Override with MACOSX_ARCH=x64|arm64|all (default: host arch)
resolve_macosx_arch() {
    if [ -n "$MACOSX_ARCH" ]; then
        case "$MACOSX_ARCH" in
            x64|arm64|all) ;;
            *)
                log_error "Invalid MACOSX_ARCH='$MACOSX_ARCH' (expected x64, arm64, or all)"
                exit 1
                ;;
        esac
        return
    fi
    if [ "$(uname -m)" = "arm64" ]; then
        MACOSX_ARCH="arm64"
    else
        MACOSX_ARCH="x64"
    fi
}

# Function to build PCL 3rdparty libraries
build_pcl_3rdparty() {
    # Check if PCL cache was restored
    if [ "$PCL_CACHE_HIT" = "true" ]; then
        log_success "PCL 3rdparty libraries restored from cache, skipping build"
        return 0
    fi
    
    log_info "Building PCL 3rdparty libraries for $PLATFORM..."
    
    cd "$PCL_PATH/src/3rdparty"
    
    case "$PLATFORM" in
        linux)
            if [ -f "linux/make-3rdparty.sh" ]; then
                cd linux
                bash ./make-3rdparty.sh
            else
                log_error "PCL 3rdparty build script not found for Linux"
                exit 1
            fi
            ;;
        macosx)
            resolve_macosx_arch
            cd macosx
            build_macos_3rdparty_arch() {
                local arch="$1"
                local script="make-3rdparty-${arch}.sh"
                if [ ! -f "$script" ]; then
                    log_error "PCL 3rdparty build script not found: $script"
                    exit 1
                fi
                export PCLLIBDIR64="$PCLDIR/lib/macosx/${arch}"
                export PCLLIBDIR="$PCLLIBDIR64"
                mkdir -p "$PCLLIBDIR64"
                log_info "Building PCL 3rdparty (${arch}) -> $PCLLIBDIR64"
                bash "./$script"
            }
            if [ "$MACOSX_ARCH" = "all" ]; then
                build_macos_3rdparty_arch x64
                build_macos_3rdparty_arch arm64
            else
                build_macos_3rdparty_arch "$MACOSX_ARCH"
            fi
            ;;
        windows)
            if [ -f "windows/make-3rdparty.bat" ]; then
                cd windows
                cmd //c make-3rdparty.bat
            else
                log_error "PCL 3rdparty build script not found for Windows"
                exit 1
            fi
            ;;
    esac
    
    log_success "PCL 3rdparty libraries built successfully"
}

# Function to ensure PCL Windows vc17 project files exist.
# Upstream PCL gitignores PCL.* under windows/, so fresh clones need generation.
generate_pcl_vc17() {
    if [ "$PLATFORM" != "windows" ]; then
        return 0
    fi
    
    log_info "Checking for PCL vc17 files..."
    
    PCL_VC16_DIR="$PCL_PATH/src/pcl/windows/vc16"
    PCL_VC17_DIR="$PCL_PATH/src/pcl/windows/vc17"
    GENERATE_PCL_VCXPROJ="$REPO_ROOT/.github/scripts/generate_pcl_vcxproj.py"
    
    if [ -d "$PCL_VC17_DIR" ] && [ -f "$PCL_VC17_DIR/PCL.vcxproj" ]; then
        mkdir -p "$PCL_VC17_DIR/x64/Release" "$PCL_VC17_DIR/x64/Debug"
        log_success "PCL vc17 files already exist"
        return 0
    fi

    # Preferred: generate from PCL sources (matches MakefileGenerator source set)
    if [ -f "$GENERATE_PCL_VCXPROJ" ]; then
        log_info "Generating PCL vc17 project from sources (PCL.vcxproj is gitignored upstream)..."
        python3 "$GENERATE_PCL_VCXPROJ" --pcl-path="$PCL_PATH"
        if [ ! -f "$PCL_VC17_DIR/PCL.vcxproj" ]; then
            log_error "Failed to generate $PCL_VC17_DIR/PCL.vcxproj"
            exit 1
        fi
        log_success "PCL vc17 files generated successfully"
        return 0
    fi

    # Legacy fallback: convert vc16 -> vc17 when present
    if [ -f "$PCL_VC16_DIR/PCL.vcxproj" ]; then
        log_info "Generating PCL vc17 files from vc16..."
        mkdir -p "$PCL_VC17_DIR"
        sed -e 's/ToolsVersion="16\.0"/ToolsVersion="17.0"/g' \
            -e 's/<PlatformToolset>v142<\/PlatformToolset>/<PlatformToolset>v143<\/PlatformToolset>/g' \
            -e 's/Windows\/vc16/Windows\/vc17/g' \
            "$PCL_VC16_DIR/PCL.vcxproj" > "$PCL_VC17_DIR/PCL.vcxproj"
        if [ -f "$PCL_VC16_DIR/PCL.vcxproj.filters" ]; then
            cp "$PCL_VC16_DIR/PCL.vcxproj.filters" "$PCL_VC17_DIR/PCL.vcxproj.filters"
        fi
        mkdir -p "$PCL_VC17_DIR/x64/Release" "$PCL_VC17_DIR/x64/Debug"
        log_success "PCL vc17 files generated from vc16"
        return 0
    fi

    log_error "Unable to create PCL Windows vc17 project under $PCL_PATH/src/pcl/windows"
    exit 1
}

# Function to build PCL core library (excludes file-formats and processes modules)
build_pcl() {
    # Check if PCL cache was restored
    if [ "$PCL_CACHE_HIT" = "true" ]; then
        log_success "PCL core library restored from cache, skipping build"
        return 0
    fi
    
    log_info "Building PCL core library for $PLATFORM..."
    
    case "$PLATFORM" in
        linux)
            cd "$PCL_PATH/src/pcl/linux/g++"
            make -f makefile-x64 -j$(nproc)
            ;;
        macosx)
            resolve_macosx_arch
            cd "$PCL_PATH/src/pcl/macosx/g++"
            build_macos_pcl_arch() {
                local arch="$1"
                export PCLLIBDIR64="$PCLDIR/lib/macosx/${arch}"
                export PCLLIBDIR="$PCLLIBDIR64"
                mkdir -p "$PCLLIBDIR64"
                log_info "Building PCL core (${arch}) -> $PCLLIBDIR64"
                make -f "makefile-${arch}" -j$(sysctl -n hw.ncpu)
            }
            if [ "$MACOSX_ARCH" = "all" ]; then
                build_macos_pcl_arch x64
                build_macos_pcl_arch arm64
            else
                build_macos_pcl_arch "$MACOSX_ARCH"
            fi
            ;;
        windows)
            # Ensure vc17 files exist
            generate_pcl_vc17
            
            cd "$PCL_PATH/src/pcl/windows/vc17"
            # Use MSBuild to build PCL (use MSBUILD_CMD if set, otherwise try msbuild)
            if [ -n "$MSBUILD_CMD" ]; then
                "$MSBUILD_CMD" PCL.vcxproj //p:Configuration=Release //p:Platform=x64 || {
                    log_error "MSBuild not found or PCL build failed"
                    log_info "Please ensure Visual Studio 2022+ is installed with C++ tools"
                    exit 1
                }
            else
                # Fallback: try msbuild from PATH
                msbuild PCL.vcxproj //p:Configuration=Release //p:Platform=x64 || {
                    log_error "MSBuild not found or PCL build failed"
                    log_info "Please ensure Visual Studio 2022+ is installed with C++ tools"
                    exit 1
                }
            fi
            ;;
    esac
    
    log_success "PCL core library built successfully"
}

# Function to generate build files
generate_build_files() {
    log_info "Generating build files for $PLATFORM..."
    
    cd "$REPO_ROOT"
    python3 "$REPO_ROOT/.github/scripts/generate_build_files.py" --platform="$PLATFORM" --repo-root="$REPO_ROOT"
    
    if [ $? -ne 0 ]; then
        log_error "Failed to generate build files"
        exit 1
    fi
    
    log_success "Build files generated successfully"
}

# Function to build the module
build_module() {
    log_info "Building VeraLuxPixInsight module for $PLATFORM..."
    
    case "$PLATFORM" in
        linux)
            cd "$REPO_ROOT/linux/g++"
            
            # Create output directories
            mkdir -p x64/Release/src/core
            mkdir -p x64/Release/src/processes/hypermetric
            mkdir -p x64/Release/src/processes/starcomposer
            
            # Build the module
            make -f makefile-x64 -j$(nproc)
            ;;
        macosx)
            cd "$REPO_ROOT/macosx/g++"
            resolve_macosx_arch
            
            build_macos_arch() {
                local arch="$1"
                export PCLLIBDIR64="$PCLDIR/lib/macosx/${arch}"
                export PCLLIBDIR="$PCLLIBDIR64"
                mkdir -p "$PCLLIBDIR64"
                mkdir -p "${arch}/Release/src/core"
                mkdir -p "${arch}/Release/src/processes/hypermetric"
                mkdir -p "${arch}/Release/src/processes/starcomposer"
                log_info "Building macOS ${arch} module target (PCLLIBDIR64=$PCLLIBDIR64)"
                make -f "makefile-${arch}" -j$(sysctl -n hw.ncpu)
            }
            
            if [ "$MACOSX_ARCH" = "all" ]; then
                build_macos_arch x64
                build_macos_arch arm64
            else
                build_macos_arch "$MACOSX_ARCH"
            fi
            ;;
        windows)
            cd "$REPO_ROOT/windows/vc17"
            
            # Build using MSBuild (use MSBUILD_CMD if set, otherwise try msbuild)
            if [ -n "$MSBUILD_CMD" ]; then
                "$MSBUILD_CMD" VeraLuxPixInsight.vcxproj //p:Configuration=Release //p:Platform=x64
            else
                # Fallback: try msbuild from PATH
                msbuild VeraLuxPixInsight.vcxproj //p:Configuration=Release //p:Platform=x64
            fi
            ;;
    esac
    
    if [ $? -ne 0 ]; then
        log_error "Module build failed"
        exit 1
    fi
    
    log_success "Module built successfully"
}

# Function to verify output
verify_output() {
    log_info "Verifying build output..."

    verify_one_binary() {
        local binary="$1"
        local expected_arch="$2"

        if [ ! -f "$binary" ]; then
            log_error "Binary not found: $binary"
            exit 1
        fi

        local size
        size=$(ls -lh "$binary" | awk '{print $5}')
        log_success "Binary created: $binary ($size)"

        if command -v file &> /dev/null; then
            local file_info
            file_info=$(file "$binary")
            log_info "$file_info"
            if [ -n "$expected_arch" ] && ! echo "$file_info" | grep -Eq "$expected_arch"; then
                log_error "Binary architecture mismatch (expected to match: $expected_arch)"
                exit 1
            fi
        fi
    }
    
    case "$PLATFORM" in
        linux)
            BINARY="$REPO_ROOT/bin/linux/VeraLuxPixInsight-pxm.so"
            verify_one_binary "$BINARY" "x86-64|x86_64|ELF 64-bit"
            ;;
        macosx)
            resolve_macosx_arch
            if [ "$MACOSX_ARCH" = "all" ]; then
                verify_one_binary "$REPO_ROOT/bin/macosx/x64/VeraLuxPixInsight-pxm.dylib" "x86_64"
                verify_one_binary "$REPO_ROOT/bin/macosx/arm64/VeraLuxPixInsight-pxm.dylib" "arm64"
                BINARY="$REPO_ROOT/bin/macosx/arm64/VeraLuxPixInsight-pxm.dylib"
            else
                BINARY="$REPO_ROOT/bin/macosx/${MACOSX_ARCH}/VeraLuxPixInsight-pxm.dylib"
                if [ "$MACOSX_ARCH" = "arm64" ]; then
                    verify_one_binary "$BINARY" "arm64"
                else
                    verify_one_binary "$BINARY" "x86_64"
                fi
            fi
            ;;
        windows)
            BINARY="$REPO_ROOT/bin/windows/VeraLuxPixInsight-pxm.dll"
            verify_one_binary "$BINARY" ""
            ;;
    esac
}

# Main build process
main() {
    echo ""
    echo "======================================================================"
    echo "VeraLuxPixInsight Build Script"
    echo "======================================================================"
    echo ""
    
    # Step 1: Check dependencies
    check_dependencies
    
    # Step 2: Clone PCL if needed
    clone_pcl

    # Resolve macOS arch before env setup so PCLLIBDIR64 is correct
    if [ "$PLATFORM" = "macosx" ]; then
        resolve_macosx_arch
        log_info "macOS build architecture: $MACOSX_ARCH"
    fi
    
    # Step 3: Set up PCL environment
    source_pcl_env
    
    # Step 4: Build PCL 3rdparty libraries
    build_pcl_3rdparty
    
    # Step 5: Build PCL core library (excludes file-formats and processes modules)
    build_pcl
    
    # Step 6: Generate build files
    generate_build_files
    
    # Step 7: Build the module
    build_module
    
    # Step 8: Verify output
    verify_output
    
    echo ""
    echo "======================================================================"
    log_success "Build completed successfully!"
    echo "======================================================================"
    echo ""
    log_info "Binary location: $BINARY"
    echo ""
}

# Run main function
main
