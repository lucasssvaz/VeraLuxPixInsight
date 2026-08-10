# VeraLux PixInsight Module

**Photometric Image Processing Suite for PixInsight**

## Overview

VeraLux is a professional image processing module for PixInsight that implements scientifically accurate photometric algorithms. This is a C++/PCL port of the original Python implementation by Riccardo Paterniti. You can find the original Python implementation [here](https://gitlab.com/free-astro/siril-scripts/-/tree/main/VeraLux).

For more information about how the repository is organized, please refer to the [PixInsight Update Repository Reference](https://pixinsight.com/doc/docs/PIRepositoryReference/PIRepositoryReference.html).

### Supported Scripts

These are the versions of the scripts that are implemented in this module:

- HyperMetric Stretch: v1.5.0
- StarComposer: v2.0.2
- Silentium: To be added
- Alchemy: To be added
- Vectra: To be added
- Revela: To be added

## Installation

1. In PixInsight, go to **Resources → Updates → Manage Repositories**.
2. Click Add and paste the URL: `https://raw.githubusercontent.com/lucasssvaz/VeraLuxPixInsight/main/dist/`
3. Click **OK**.
4. Go to **Resources → Updates → Check for Updates**.
5. Install the package and restart PixInsight to complete the installation.

## Processes

### HyperMetric Stretch (HMS)

A precision linear-to-nonlinear stretching engine designed to maximize sensor fidelity while managing the transition to the visible domain.

**Key Features:**
- Inverse hyperbolic sine (arcsinh) based stretching
- Sensor-specific quantum efficiency weighting (27 camera profiles)
- Vector color preservation ("True Color" methodology)
- Dual processing modes (Ready-to-Use / Scientific)
- Adaptive black point detection
- Real-time preview with instant parameter feedback

**Implementation Validation:**

This C++ implementation has been rigorously validated against the original Python version:

- ✅ **Mathematical Accuracy:** All core formulas match exactly (< 1e-6 error)
- ✅ **Sensor Profiles:** All 27 profiles verified (identical weights)
- ✅ **Processing Pipeline:** Complete algorithm parity confirmed
- ✅ **Default Behavior:** Uses exact percentiles for perfect Python match
- ✅ **Performance Option:** Optional MAD approximations for improved performance

By default, the C++ port uses **exact percentiles** matching the Python implementation perfectly. For improved performance while maintaining accuracy (less than 0.001 typical error), generate the makefiles/Visual Studio projects with `HMS_USE_MAD=1` defined to enable statistically robust MAD approximations for Linear Expansion and Adaptive Scaling bounds. All 5 core mathematical function tests passed validation.

**Processing Modes:**

1. **Ready-to-Use Mode:**
   - Aesthetic, export-ready output
   - Automatic output scaling and MTF
   - Soft-clipping for highlights
   - Unified Color Strategy slider

2. **Scientific Mode:**
   - Mathematically pure output
   - Manual linear expansion control
   - Independent color grip and shadow convergence
   - Ideal for mosaics and further processing

**Requirements:**
- Linear input data (not stretched)
- Color calibrated RGB (SPCC recommended)
- Background gradients removed

**Usage:**

1. Ensure the image is **linear** and **color calibrated** (e.g., SPCC) and background-corrected
2. Launch **Process → VeraLux → HyperMetric Stretch**
3. Select a **Sensor Profile**. If your sensor is unknown, use **Rec.709 (Recommended)**
4. Choose **Processing Mode**: **Ready-to-Use** for an export-ready stretch or **Scientific** for a controlled physically consistent output suitable for further tone mapping
5. Enable **Adaptive Anchor** unless you have strong gradients/unusual background conditions where the conservative statistical anchor is preferable
6. **In Ready-to-Use mode:** set **Target Bg** and click **Auto-Calc** to solve **Log D**. Adjust **Color Strategy** as needed
7. **In Scientific mode:** click **Auto-Calc** to get a good starting **Log D**, then tune **Protect b**, **Color Conv**, and optionally **Linear Expan**, **Color Grip**, and **Shadow Conv**
8. Enable real-time preview to fine-tune parameters
9. Click apply when satisfied with the result. It is recommended to slowly iterate the process until the result is satisfactory.

### StarComposer

A specialized photometric reconstruction engine for deep-sky astrophotography that solves star bloating and bleaching issues.

**Key Features:**
- Hybrid Scalar/Vector engine for white cores with color halos
- Star Surgery operations (LSR, Optical Healing, Morphological Reduction)
- Dual composition modes (Screen, Linear Add)
- Sensor-specific quantum efficiency weighting (27 camera profiles)
- Signal conditioning with Gamma 2.4 and micro-blur

**Implementation Validation:**

This C++ implementation has been rigorously validated against the original Python version:

- ✅ **Mathematical Accuracy:** All core formulas match exactly (< 1e-6 error)
- ✅ **Hybrid Engine:** Scalar and Vector modes verified independently
- ✅ **Surgery Operations:** All operations validated against Python reference
- ✅ **Default Behavior:** Uses exact algorithms for perfect Python match
- ✅ **Performance Option:** Optional optimizations for 2-3x speedup

By default, the C++ port uses **exact algorithms** matching the Python implementation perfectly. For improved performance while maintaining accuracy (< 0.001 typical error), compile with optimization flags enabled (`SCS_USE_FAST_BLUR`, `SCS_USE_APPROX_YCRCB`).

**Requirements:**
- Linear starmask image (from StarNet/StarXTerminator)
- Stretched starless image (non-linear background)
- Both images should be RGB and color calibrated

**Usage:**

1. Launch **Process → VeraLux → StarComposer**
2. Select **Linear Starmask** view (must be unstretched for correct color reconstruction)
3. Select **Stretched Starless** view (already processed background)
4. Choose **Composition Mode**: **Screen** (safe, no clipping) or **Linear Add** (physical accuracy)
5. Select **Sensor Profile** matching your camera (or use Rec.709 if unknown)
6. Adjust **Star Intensity (Log D)** to control star brightness
7. Adjust **Profile Hardness (b)** to control star geometry (higher = sharper)
8. Enable **Adaptive Anchor** for best contrast (recommended)
9. Fine-tune **Color Grip** (0% = crisp white cores, 100% = maximum color preservation)
10. Optional: Apply **Star Surgery** operations if needed:
    - **Core Rejection (LSR)**: Remove large structures like galaxy cores
    - **Optical Healing**: Fix chromatic aberration halos
    - **Morphological Reduction**: Shrink star diameters
11. Click apply to process and create the composed result

## Building

The module uses an automated build system that generates makefiles and Visual Studio projects without requiring PixInsight's MakefileGenerator. The build process is fully automated via GitHub Actions and can also be run locally.

Toolchain requirements follow the current [PCL](https://gitlab.com/pixinsight/PCL) release (C++20): GCC 12+ on Linux, Xcode 26 / Clang 17+ on macOS 15+, and Visual C++ 2022 on Windows. See PCL's [Supported Compilers](https://gitlab.com/pixinsight/PCL/-/blob/master/README.md#supported-compilers) section for the official reference builds.

### Automated CI Build

The module is automatically built on all platforms (Linux, macOS, Windows) when changes are pushed to the repository. The compiled binaries are automatically committed to the `bin/` directory. On macOS, CI builds both Intel (`x64`) and Apple Silicon (`arm64`).

**Workflow:**
1. Push changes to `src/`, `doc/`, or `rsc/` directories
2. GitHub Actions automatically builds for all platforms
3. Binaries are committed to `bin/linux/`, `bin/macosx/x64/`, `bin/macosx/arm64/`, and `bin/windows/`

### Local Build

**Prerequisites:**
- Clone the [PCL repository](https://gitlab.com/pixinsight/PCL) (will be auto-cloned if not present)
- `python3` and `git` on all hosts
- Platform-specific tools (from the PCL README):
  - **Linux**: Ubuntu 22.04 LTS (or equivalent) with **GCC 12 or later**, GNU make, and the usual build packages (`build-essential`, `pkg-config`)
  - **macOS**: **macOS 15 or later** with **Xcode 26** (Clang 17.0 or later). Official PCL reference builds use macOS 26 + Xcode 26.x for arm64 and macOS 15 + Xcode 26.x for x64
  - **Windows**: **Visual Studio 2022** with the C++ desktop workload (Visual C++ 2022 / `vc17` only, as of PCL 2.10)

**Build Instructions:**

```bash
# Clone the repository
git clone https://github.com/lucasssvaz/VeraLuxPixInsight.git
cd VeraLuxPixInsight

# Run the build script (auto-detects platform)
./build.sh

# Or specify platform explicitly
./build.sh --platform=macosx

# On macOS, choose architecture (default: host arch)
MACOSX_ARCH=arm64 ./build.sh --platform=macosx   # Apple Silicon only
MACOSX_ARCH=x64 ./build.sh --platform=macosx     # Intel only
MACOSX_ARCH=all ./build.sh --platform=macosx     # both (as in CI)

# Or specify PCL location
./build.sh --platform=linux --pcl-path=/path/to/PCL
```

The build script will:
1. Install required dependencies for your platform
2. Clone PCL from GitLab if not present
3. Build PCL 3rdparty libraries
4. Build PCL itself
5. Generate build files (makefiles/vcxproj)
6. Build the VeraLuxPixInsight module
7. Place binaries under `bin/{platform}/` (macOS uses `bin/macosx/{x64|arm64}/`)

**Output:**
- Linux: `bin/linux/VeraLuxPixInsight-pxm.so`
- macOS (Intel): `bin/macosx/x64/VeraLuxPixInsight-pxm.dylib`
- macOS (Apple Silicon): `bin/macosx/arm64/VeraLuxPixInsight-pxm.dylib`
- Windows: `bin/windows/VeraLuxPixInsight-pxm.dll`

### Module Signing (Manual)

After building, modules must be signed before they can be installed in PixInsight. Omit `--module-file` to sign every built binary under `bin/` and, when present, embed a repository `Signature` in `dist/updates.xri` via PixInsight’s `--sign-xml-file`:

```bash
./sign_module.sh \
  --xssk-file=/path/to/your/key.xssk \
  --xssk-password=yourpassword
```

Or sign one binary (use `--no-xri` to skip the manifest):

```bash
./sign_module.sh \
  --module-file=bin/macosx/arm64/VeraLuxPixInsight-pxm.dylib \
  --xssk-file=/path/to/your/key.xssk \
  --xssk-password=yourpassword \
  --no-xri
```

### Creating Release Packages

After signing the binaries, you can create distribution packages:

1. Ensure signed binaries (and `.xsgn` files) are present for Linux, macOS x64, macOS arm64, and Windows under `bin/`
2. Trigger the package workflow manually via GitHub Actions UI with version number, or run the packager locally
3. Re-run `./sign_module.sh` so `dist/updates.xri` is signed (packaging regenerates an unsigned manifest)
4. Commit `dist/` and publish a GitHub release if desired

Or run locally:

```bash
python .github/scripts/package_release.py --version=0.1.0
./sign_module.sh --xssk-file=/path/to/your/key.xssk --xssk-password=yourpassword
```

This creates one `.zip` package per OS/arch (`linux/x64`, `macosx/x64`, `macosx/arm64`, `windows/x64`) and generates the `updates.xri` manifest.

### Documentation

The documentation is generated using PIDoc. You can find the documentation reference [here](https://pixinsight.com/doc/docs/PIDocReference/PIDocReference.html). The documentation was compiled on macOS. Other platforms might require some adjustments.

1. Make sure you have the required tools installed and `latex`, `dvips`, `epstopdf`, `pdf2svg`, `dvisvgm` and `convert` available in your path.
2. Generate a new documentation system in **Scripts → Development → DocumentationCompiler** by selecting the **Generate new PIDoc system** option. Select an empty directory for the system.
3. In the same screen, Add the `.pidoc` files that you want to compile from the [doc/src](doc/src) directory.
4. Select the documentation system folder you created in the previous step.
5. Click the **Run** button to compile the documentation.
6. You can view the compiled HTML documentation in the `tools` folder of the documentation system you created in the previous step.

## Sensor Profiles

The module includes 27 sensor profiles derived from SPCC data:

- Standard: Rec.709
- Sony: IMX571, IMX455, IMX410, IMX269, IMX294, IMX533, IMX676, IMX585, IMX662, IMX678, IMX462, IMX715, IMX482, IMX183, IMX178, IMX224
- Canon EOS: Modern (60D/600D/500D), Legacy (300D/40D/20D)
- Nikon DSLR: Modern (D5100/D7200), Legacy (D3/D300/D90)
- Fujifilm X-Trans 5 HR
- Panasonic MN34230
- ZWO Seestar S50, S30
- Narrowband: HOO, SHO

## Credits

**Original Algorithm:**
- Author: Riccardo Paterniti (2025)
- Contact: info@veralux.space
- Project: VeraLux (Siril Python implementation)

**PixInsight Port:**
- Author: Lucas Saavedra Vaz (2026)
- Framework: PixInsight Class Library (PCL) 2.10.4

**Scientific Foundation:**
- Inspired by Dr. Roger N. Clark's "True Color" methodology
- Math basis: Inverse Hyperbolic Stretch (IHS)
- Sensor science: Hardware-specific Quantum Efficiency weighting

## Acknowledgments

This C++ implementation for PixInsight would not exist without the foundational work of:

**Original Algorithm & Methodology:**
- **Riccardo Paterniti** - Creator of the VeraLux suite for Siril.
- Website: https://veralux.space
- Email: info@veralux.space

**Special Thanks:**
- **killerciao** ([VeraLuxPorting](https://github.com/killerciao/VeraLuxPorting)) - For the JavaScript/PJSR port, which inspired the creation of this native process module for PixInsight

Special thanks to the PixInsight community and the open-source ethos that makes projects like this possible.

## License

GNU General Public License v3.0

Copyright (c) 2026 Lucas Saavedra Vaz (C++ Port for PixInsight) \
Copyright (c) 2025 Riccardo Paterniti (Original Python implementation)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

See LICENSE file for full text.

## Support

For questions about this PixInsight module, please open an issue or discussion in the GitHub repository.

For questions about the original algorithm:
- Email: info@veralux.space
- Website: https://veralux.space

## Version History

### 0.1.1 (August 2026)
- Add native macOS Apple Silicon (`arm64`) builds alongside Intel (`x64`)
- Align toolchain with PCL 2.10.4 (GCC 12+, Xcode 26, Visual C++ 2022)
- Ship separate XRI packages per OS/architecture

### 0.1.0 (January 2026)
- Initial PixInsight PCL port
- Full feature parity with Python version
- Standard PixInsight GUI
- Real-time preview support
- 27 sensor profiles

## Future Development

Planned additional VeraLux processes:
- Silentium (Noise Reduction)
- Alchemy (Color Grading)
- Vectra (Gradient Removal)
- Revela (Advanced Processing)

All will share the core VeraLuxEngine, StarEngine, and SensorProfiles.
