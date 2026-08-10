#!/usr/bin/env python3
"""
PixInsight Module Release Packager
Creates zip packages and updates.xri manifest for PixInsight module distribution.

macOS ships separate x64 and arm64 packages (modern XRI practice; see GraXpert).
"""

import argparse
import hashlib
import sys
import zipfile
from datetime import datetime
from pathlib import Path

MODULE_NAME = "VeraLuxPixInsight"
MODULE_TITLE = "VeraLux PixInsight Module"

# (os_name, arch, binary_relpath) — arch is the XRI arch= attribute
RELEASE_TARGETS = [
    ("linux", "x64", Path("bin/linux") / f"{MODULE_NAME}-pxm.so"),
    ("macosx", "x64", Path("bin/macosx/x64") / f"{MODULE_NAME}-pxm.dylib"),
    ("macosx", "arm64", Path("bin/macosx/arm64") / f"{MODULE_NAME}-pxm.dylib"),
    ("windows", "x64", Path("bin/windows") / f"{MODULE_NAME}-pxm.dll"),
]


def calculate_sha1(file_path):
    """Calculate SHA1 hash of a file"""
    sha1 = hashlib.sha1()
    with open(file_path, "rb") as f:
        while chunk := f.read(8192):
            sha1.update(chunk)
    return sha1.hexdigest()


def get_module_description(readme_path):
    """Extract module description from README.md"""
    try:
        with open(readme_path, "r", encoding="utf-8") as f:
            lines = f.readlines()

        description = []
        in_overview = False
        for line in lines:
            if "## Overview" in line:
                in_overview = True
                continue
            if in_overview:
                if line.startswith("##"):
                    break
                if line.strip() and not line.startswith("#"):
                    description.append(line.strip())

        if description:
            return " ".join(description[:3])
        return (
            "VeraLux is a professional image processing module for PixInsight that "
            "implements scientifically accurate photometric algorithms."
        )
    except Exception:
        return "VeraLux PixInsight Module"


def create_package(os_name, arch, binary_relpath, version, repo_root, dist_dir):
    """Create a zip package for a specific OS/arch"""
    binary_path = repo_root / binary_relpath
    sign_path = binary_path.parent / f"{MODULE_NAME}-pxm.xsgn"
    binary_name = binary_path.name

    if not binary_path.exists():
        print(f"Error: Binary not found: {binary_path}")
        return None

    if not sign_path.exists():
        print(f"Error: Signature file not found: {sign_path}")
        return None

    date_stamp = datetime.now().strftime("%Y%m%d")
    package_name = f"{MODULE_NAME}-{os_name}-{arch}-{version}-{date_stamp}.zip"
    package_path = dist_dir / package_name

    print(f"\nCreating package for {os_name}/{arch}...")
    print(f"  Binary: {binary_path}")
    print(f"  Signature: {sign_path}")
    print(f"  Package: {package_name}")

    with zipfile.ZipFile(package_path, "w", zipfile.ZIP_DEFLATED) as zip_file:
        # Install layout is always bin/<module> regardless of source arch subdir
        zip_file.write(binary_path, arcname=f"bin/{binary_name}")
        zip_file.write(sign_path, arcname=f"bin/{MODULE_NAME}-pxm.xsgn")

        rsc_dir = repo_root / "rsc"
        if rsc_dir.exists():
            for svg_file in rsc_dir.rglob("*.svg"):
                arcname = f"rsc/{svg_file.relative_to(rsc_dir)}"
                zip_file.write(svg_file, arcname=arcname)
                print(f"  Added: {arcname}")

        doc_tools_dir = repo_root / "doc" / "tools"
        if doc_tools_dir.exists():
            for doc_file in doc_tools_dir.rglob("*"):
                if doc_file.is_file():
                    arcname = f"doc/tools/{doc_file.relative_to(doc_tools_dir)}"
                    zip_file.write(doc_file, arcname=arcname)
                    print(f"  Added: {arcname}")

    sha1 = calculate_sha1(package_path)
    file_size = package_path.stat().st_size

    print(f"  Size: {file_size / 1024:.2f} KB")
    print(f"  SHA1: {sha1}")

    return {
        "os": os_name,
        "arch": arch,
        "filename": package_name,
        "sha1": sha1,
        "size": file_size,
        "releaseDate": date_stamp,
    }


def generate_updates_xri(packages, version, min_version, max_version, repo_root, dist_dir):
    """Generate updates.xri manifest file"""
    print("\nGenerating updates.xri...")

    readme_path = repo_root / "README.md"
    description = get_module_description(readme_path)

    xml_lines = ['<?xml version="1.0" encoding="UTF-8"?>']
    xml_lines.append('<xri version="1.0">')
    xml_lines.append("  <description>")
    xml_lines.append("    <p>")
    xml_lines.append(f"      <b>{MODULE_TITLE} Repository</b>")
    xml_lines.append("    </p>")
    xml_lines.append("    <p>")
    xml_lines.append(f"      {description}")
    xml_lines.append("    </p>")
    xml_lines.append("  </description>")
    xml_lines.append("")

    for pkg in packages:
        xml_lines.append(
            f'  <platform os="{pkg["os"]}" arch="{pkg["arch"]}" '
            f'version="{min_version}:{max_version}">'
        )
        xml_lines.append(
            f'    <package fileName="{pkg["filename"]}" sha1="{pkg["sha1"]}" '
            f'type="module" releaseDate="{pkg["releaseDate"]}">'
        )
        xml_lines.append("      <title>")
        xml_lines.append(f"        {MODULE_TITLE} v{version} ({pkg['os']}/{pkg['arch']})")
        xml_lines.append("      </title>")
        xml_lines.append("      <description>")
        xml_lines.append("        <p>")
        xml_lines.append(
            f"          This update installs the {MODULE_TITLE} version {version} "
            f"for {pkg['os']} ({pkg['arch']})"
        )
        xml_lines.append("        </p>")
        xml_lines.append("        <p>")
        xml_lines.append(
            "          Copyright (c) 2026 Lucas Saavedra Vaz (C++ Port for PixInsight)"
        )
        xml_lines.append("        </p>")
        xml_lines.append("        <p>")
        xml_lines.append(
            "          Copyright (c) 2025 Riccardo Paterniti (Original Python implementation)"
        )
        xml_lines.append("        </p>")
        xml_lines.append("        <p>")
        xml_lines.append(
            "          This program is free software: you can redistribute it and/or modify"
        )
        xml_lines.append("        </p>")
        xml_lines.append("        <p>")
        xml_lines.append(
            "          it under the terms of the GNU General Public License as published by"
        )
        xml_lines.append("        </p>")
        xml_lines.append("        <p>")
        xml_lines.append(
            "          the Free Software Foundation, either version 3 of the License, or"
        )
        xml_lines.append("        </p>")
        xml_lines.append("        <p>")
        xml_lines.append("          (at your option) any later version.")
        xml_lines.append("        </p>")
        xml_lines.append("      </description>")
        xml_lines.append("    </package>")
        xml_lines.append("  </platform>")
        xml_lines.append("")

    xml_lines.append("</xri>")
    xml_lines.append("")

    xri_path = dist_dir / "updates.xri"
    with open(xri_path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(xml_lines))

    print(f"  Created: {xri_path}")
    print(
        "  Targets: "
        + ", ".join(f"{p['os']}/{p['arch']}" for p in packages)
    )
    return xri_path


def main():
    parser = argparse.ArgumentParser(
        description="Package VeraLuxPixInsight module for release"
    )
    parser.add_argument("--version", required=True, help="Module version (e.g., 0.1.0)")
    parser.add_argument(
        "--min-pi-version", default="1.8.9", help="Minimum PixInsight version"
    )
    parser.add_argument(
        "--max-pi-version", default="1.9.99", help="Maximum PixInsight version"
    )
    parser.add_argument(
        "--platform",
        choices=["linux", "macosx", "windows", "all"],
        default="all",
        help="Target OS (macosx includes both x64 and arm64)",
    )
    parser.add_argument("--repo-root", default=None, help="Repository root path")

    args = parser.parse_args()

    if args.repo_root:
        repo_root = Path(args.repo_root).resolve()
    else:
        script_dir = Path(__file__).parent
        repo_root = script_dir.parent.parent

    print("======================================================================")
    print(f"{MODULE_TITLE} Release Packager")
    print("======================================================================")
    print(f"Repository root: {repo_root}")
    print(f"Version: {args.version}")
    print(f"PixInsight version range: {args.min_pi_version}:{args.max_pi_version}")

    dist_dir = repo_root / "dist"
    dist_dir.mkdir(exist_ok=True)

    if args.platform == "all":
        targets = RELEASE_TARGETS
    else:
        targets = [t for t in RELEASE_TARGETS if t[0] == args.platform]

    packages = []
    for os_name, arch, binary_relpath in targets:
        pkg = create_package(
            os_name, arch, binary_relpath, args.version, repo_root, dist_dir
        )
        if pkg:
            packages.append(pkg)
        else:
            print(f"Error: Failed to create package for {os_name}/{arch}")
            sys.exit(1)

    if not packages:
        print("Error: No packages created")
        sys.exit(1)

    generate_updates_xri(
        packages,
        args.version,
        args.min_pi_version,
        args.max_pi_version,
        repo_root,
        dist_dir,
    )

    print("\n======================================================================")
    print("✓ Release packaging completed successfully!")
    print("======================================================================")
    print(f"\nCreated {len(packages)} package(s):")
    for pkg in packages:
        print(f"  • {pkg['filename']} ({pkg['os']}/{pkg['arch']})")
    print(f"\nDistribution directory: {dist_dir}")
    print("")


if __name__ == "__main__":
    main()
