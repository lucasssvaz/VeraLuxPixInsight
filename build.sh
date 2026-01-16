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
            log_info "Checking for required build tools..."

            # Check for essential tools
            missing_tools=""
            if ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null; then
                missing_tools="$missing_tools gcc or clang"
            fi
            if ! command -v make &> /dev/null; then
                missing_tools="$missing_tools make"
            fi
            if ! command -v pkg-config &> /dev/null; then
                missing_tools="$missing_tools pkg-config"
            fi
            if ! command -v python3 &> /dev/null; then
                missing_tools="$missing_tools python3"
            fi

            if [ -n "$missing_tools" ]; then
                log_error "Missing required tools:$missing_tools"
                log_error "Please install build-essential, clang, pkg-config, and python3"
                exit 1
            else
                log_success "All required build tools are available"
            fi
            ;;
        macosx)
            if ! command -v clang &> /dev/null; then
                log_error "Xcode Command Line Tools not found"
                log_error "Please install Xcode Command Line Tools: xcode-select --install"
                exit 1
            else
                log_success "Xcode Command Line Tools found"
            fi

            # Check Xcode version
            if command -v xcodebuild &> /dev/null; then
                XCODE_VERSION=$(xcodebuild -version | head -1 | sed 's/Xcode //' | cut -d'.' -f1)
                if [[ "$XCODE_VERSION" -eq 15 ]]; then
                    log_success "Xcode version 15.x found: $(xcodebuild -version | head -1)"
                else
                    log_error "Xcode version 15.x required, but found: $(xcodebuild -version | head -1)"
                    log_error "Please install Xcode 15.x from the Mac App Store or developer.apple.com"
                    exit 1
                fi
            else
                log_error "xcodebuild command not found"
                log_error "Please ensure Xcode is properly installed"
                exit 1
            fi
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
            if [ -f "macosx/build-all.sh" ]; then
                cd macosx
                bash ./build-all.sh
            elif [ -f "macosx/make-3rdparty.sh" ]; then
                cd macosx
                bash ./make-3rdparty.sh
            else
                log_error "PCL 3rdparty build script not found for macOS"
                exit 1
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

# Function to generate vc17 files from vc16 if they don't exist
generate_pcl_vc17() {
    if [ "$PLATFORM" != "windows" ]; then
        return 0
    fi
    
    log_info "Checking for PCL vc17 files..."
    
    PCL_VC16_DIR="$PCL_PATH/src/pcl/windows/vc16"
    PCL_VC17_DIR="$PCL_PATH/src/pcl/windows/vc17"
    
    if [ ! -d "$PCL_VC16_DIR" ]; then
        log_error "PCL vc16 directory not found: $PCL_VC16_DIR"
        exit 1
    fi
    
    # Check if vc17 directory exists and has the project file
    if [ -d "$PCL_VC17_DIR" ] && [ -f "$PCL_VC17_DIR/PCL.vcxproj" ]; then
        log_success "PCL vc17 files already exist"
        return 0
    fi
    
    log_info "Generating PCL vc17 files from vc16..."
    
    # Create vc17 directory
    mkdir -p "$PCL_VC17_DIR"
    
    # Copy and convert .vcxproj file
    if [ -f "$PCL_VC16_DIR/PCL.vcxproj" ]; then
        # Convert vc16 to vc17: ToolsVersion 16.0 -> 17.0, v142 -> v143, vc16 -> vc17
        sed -e 's/ToolsVersion="16\.0"/ToolsVersion="17.0"/g' \
            -e 's/<PlatformToolset>v142<\/PlatformToolset>/<PlatformToolset>v143<\/PlatformToolset>/g' \
            -e 's/Windows\/vc16/Windows\/vc17/g' \
            "$PCL_VC16_DIR/PCL.vcxproj" > "$PCL_VC17_DIR/PCL.vcxproj"
        log_success "Generated PCL.vcxproj for vc17"
    else
        log_error "PCL.vcxproj not found in vc16 directory"
        exit 1
    fi
    
    # Copy .vcxproj.filters file (no changes needed)
    if [ -f "$PCL_VC16_DIR/PCL.vcxproj.filters" ]; then
        cp "$PCL_VC16_DIR/PCL.vcxproj.filters" "$PCL_VC17_DIR/PCL.vcxproj.filters"
        log_success "Copied PCL.vcxproj.filters for vc17"
    fi
    
    # Create x64 output directories if they don't exist
    mkdir -p "$PCL_VC17_DIR/x64/Release"
    mkdir -p "$PCL_VC17_DIR/x64/Debug"
    
    log_success "PCL vc17 files generated successfully"
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
            cd "$PCL_PATH/src/pcl/macosx/g++"
            make -f makefile-x64 -j$(sysctl -n hw.ncpu)
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
            
            # Build the module
            make -f makefile-x64 -j$(nproc)
            ;;
        macosx)
            cd "$REPO_ROOT/macosx/g++"
            
            # Create output directories
            mkdir -p x64/Release/src/core
            mkdir -p x64/Release/src/processes/hypermetric
            
            # Build the module
            make -f makefile-x64 -j$(sysctl -n hw.ncpu)
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
    
    case "$PLATFORM" in
        linux)
            BINARY="$REPO_ROOT/bin/linux/VeraLuxPixInsight-pxm.so"
            ;;
        macosx)
            BINARY="$REPO_ROOT/bin/macosx/VeraLuxPixInsight-pxm.dylib"
            ;;
        windows)
            BINARY="$REPO_ROOT/bin/windows/VeraLuxPixInsight-pxm.dll"
            ;;
    esac
    
    if [ -f "$BINARY" ]; then
        SIZE=$(ls -lh "$BINARY" | awk '{print $5}')
        log_success "Binary created: $BINARY ($SIZE)"
    else
        log_error "Binary not found: $BINARY"
        exit 1
    fi
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
