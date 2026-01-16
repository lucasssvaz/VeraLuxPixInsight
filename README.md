# VeraLux PixInsight Module

**Photometric Image Processing Suite for PixInsight**

## Overview

VeraLux is a professional image processing module for PixInsight that implements scientifically accurate photometric algorithms. This is a C++/PCL port of the original Python implementation by Riccardo Paterniti. You can find the original Python implementation [here](https://gitlab.com/free-astro/siril-scripts/-/tree/main/VeraLux).

For more information about how the repository is organized, please refer to the [PixInsight Update Repository Reference](https://pixinsight.com/doc/docs/PIRepositoryReference/PIRepositoryReference.html).

### Supported Scripts

These are the versions of the scripts that are implemented in this module:

- HyperMetric Stretch: v1.5.0
- StarComposer: To be added
- Silentium: To be added
- Alchemy: To be added
- Vectra: To be added

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

## Building

The module uses an automated build system that generates makefiles and Visual Studio projects without requiring PixInsight's MakefileGenerator. The build process is fully automated via GitHub Actions and can also be run locally.

### Automated CI Build

The module is automatically built on all platforms (Linux, macOS, Windows) when changes are pushed to the repository. The compiled binaries are automatically committed to the `bin/` directory.

**Workflow:**
1. Push changes to `src/`, `doc/`, or `rsc/` directories
2. GitHub Actions automatically builds for all platforms
3. Binaries are committed to `bin/linux/`, `bin/macosx/`, `bin/windows/`

### Local Build

**Prerequisites:**
- Clone the [PCL repository](https://gitlab.com/pixinsight/PCL) (will be auto-cloned if not present)
- Platform-specific tools:
  - **Linux**: build-essential, clang, pkg-config, python3
  - **macOS**: Xcode Command Line Tools
  - **Windows**: Visual Studio 2022+ with C++ tools

**Build Instructions:**

```bash
# Clone the repository
git clone https://github.com/lucasssvaz/VeraLuxPixInsight.git
cd VeraLuxPixInsight

# Run the build script (auto-detects platform)
./build.sh

# Or specify platform explicitly
./build.sh --platform=macosx

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
7. Place the binary in `bin/{platform}/`

**Output:**
- Linux: `bin/linux/VeraLuxPixInsight-pxm.so`
- macOS: `bin/macosx/VeraLuxPixInsight-pxm.dylib`
- Windows: `bin/windows/VeraLuxPixInsight-pxm.dll`

### Module Signing (Manual)

After building, modules must be signed before they can be installed in PixInsight:

```bash
./sign_module.sh \
  --module-file=bin/macosx/VeraLuxPixInsight-pxm.dylib \
  --xssk-file=/path/to/your/key.xssk \
  --xssk-password=yourpassword
```

Repeat for each platform binary you want to sign.

### Creating Release Packages

After signing the binaries, you can create distribution packages:

1. Ensure signed binaries are in `bin/` directory
2. Trigger the package workflow manually via GitHub Actions UI with version number
3. Packages will be created in `dist/` directory
4. Download packages and create a GitHub release

Or run locally:

```bash
python .github/scripts/package_release.py --version=0.1.0
```

This creates tar.gz packages for each platform and generates the `updates.xri` manifest file.

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
- Framework: PixInsight Class Library (PCL) 2.9.4

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

### 0.1.0 (January 2026)
- Initial PixInsight PCL port
- Full feature parity with Python version
- Standard PixInsight GUI
- Real-time preview support
- 27 sensor profiles

## Future Development

Planned additional VeraLux processes:
- Silentium (Noise Reduction)
- StarComposer (Star Synthesis)
- Alchemy (Color Grading)
- Vectra (Gradient Removal)

All will share the core VeraLuxEngine and SensorProfiles.
