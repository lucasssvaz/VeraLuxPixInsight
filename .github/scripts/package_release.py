#!/usr/bin/env python3
"""
PixInsight Module Release Packager
Creates tar.gz packages and updates.xri manifest for PixInsight module distribution
"""

import os
import sys
import argparse
import tarfile
import hashlib
from pathlib import Path
from datetime import datetime

MODULE_NAME = "VeraLuxPixInsight"
MODULE_TITLE = "VeraLux PixInsight Module"

def calculate_sha1(file_path):
    """Calculate SHA1 hash of a file"""
    sha1 = hashlib.sha1()
    with open(file_path, 'rb') as f:
        while chunk := f.read(8192):
            sha1.update(chunk)
    return sha1.hexdigest()

def get_module_description(readme_path):
    """Extract module description from README.md"""
    try:
        with open(readme_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        # Look for overview section
        description = []
        in_overview = False
        for line in lines:
            if '## Overview' in line:
                in_overview = True
                continue
            if in_overview:
                if line.startswith('##'):
                    break
                if line.strip() and not line.startswith('#'):
                    description.append(line.strip())
        
        if description:
            return ' '.join(description[:3])  # First 3 lines
        return "VeraLux is a professional image processing module for PixInsight that implements scientifically accurate photometric algorithms."
    except:
        return "VeraLux PixInsight Module"

def create_package(platform, version, repo_root, dist_dir):
    """Create a tar.gz package for a specific platform"""
    
    # Platform-specific binary extensions
    binary_ext = {
        'linux': 'so',
        'macosx': 'dylib',
        'windows': 'dll'
    }
    
    # Check if binary exists
    binary_name = f"{MODULE_NAME}-pxm.{binary_ext[platform]}"
    binary_path = repo_root / "bin" / platform / binary_name
    
    if not binary_path.exists():
        print(f"Error: Binary not found: {binary_path}")
        return None
    
    # Package filename
    date_stamp = datetime.now().strftime('%Y%m%d')
    package_name = f"{MODULE_NAME}-{platform}-{version}-{date_stamp}.tar.gz"
    package_path = dist_dir / package_name
    
    print(f"\nCreating package for {platform}...")
    print(f"  Binary: {binary_path}")
    print(f"  Package: {package_name}")
    
    # Create tar.gz with proper structure
    with tarfile.open(package_path, 'w:gz') as tar:
        # Add binary to bin/ directory
        tar.add(binary_path, arcname=f"bin/{binary_name}")
        
        # Add resource files (icons)
        rsc_dir = repo_root / "rsc"
        if rsc_dir.exists():
            for svg_file in rsc_dir.rglob("*.svg"):
                arcname = f"rsc/{svg_file.relative_to(rsc_dir)}"
                tar.add(svg_file, arcname=arcname)
                print(f"  Added: {arcname}")
    
    # Calculate SHA1
    sha1 = calculate_sha1(package_path)
    file_size = package_path.stat().st_size
    
    print(f"  Size: {file_size / 1024:.2f} KB")
    print(f"  SHA1: {sha1}")
    
    return {
        'platform': platform,
        'filename': package_name,
        'sha1': sha1,
        'size': file_size,
        'releaseDate': date_stamp
    }

def generate_updates_xri(packages, version, min_version, max_version, repo_root, dist_dir):
    """Generate updates.xri manifest file"""
    
    print("\nGenerating updates.xri...")
    
    # Get module description
    readme_path = repo_root / "README.md"
    description = get_module_description(readme_path)
    
    # Platform mapping for xri
    platform_names = {
        'linux': 'linux',
        'macosx': 'macosx',
        'windows': 'windows'
    }
    
    # Build XML content
    xml_lines = ['<?xml version="1.0" encoding="UTF-8"?>']
    xml_lines.append('<xri version="1.0">')
    xml_lines.append('  <description>')
    xml_lines.append('    <p>')
    xml_lines.append(f'      <b>{MODULE_TITLE} Repository</b>')
    xml_lines.append('    </p>')
    xml_lines.append('    <p>')
    xml_lines.append(f'      {description}')
    xml_lines.append('    </p>')
    xml_lines.append('  </description>')
    xml_lines.append('')
    
    # Create platform sections
    for pkg in packages:
        platform_name = platform_names[pkg['platform']]
        xml_lines.append(f'  <platform os="{platform_name}" arch="noarch" version="{min_version}:{max_version}">')
        xml_lines.append(f'    <package fileName="{pkg["filename"]}" sha1="{pkg["sha1"]}" type="module" releaseDate="{pkg["releaseDate"]}">')
        xml_lines.append('      <title>')
        xml_lines.append(f'        {MODULE_TITLE} v{version}')
        xml_lines.append('      </title>')
        xml_lines.append('      <description>')
        xml_lines.append('        <p>')
        xml_lines.append(f'          This update installs the {MODULE_TITLE} version {version}')
        xml_lines.append('        </p>')
        xml_lines.append('        <p>')
        xml_lines.append('          Copyright (c) 2026 Lucas Saavedra Vaz, All Rights Reserved.')
        xml_lines.append('        </p>')
        xml_lines.append('      </description>')
        xml_lines.append('    </package>')
        xml_lines.append('  </platform>')
    
    xml_lines.append('</xri>')
    xml_lines.append('')  # Final newline
    
    # Write XRI file with UTF-8 encoding (no BOM)
    xri_path = dist_dir / "updates.xri"
    with open(xri_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(xml_lines))
    
    print(f"  Created: {xri_path}")
    print(f"  Platforms: {', '.join([p['platform'] for p in packages])}")
    
    return xri_path

def main():
    parser = argparse.ArgumentParser(description='Package VeraLuxPixInsight module for release')
    parser.add_argument('--version', required=True, help='Module version (e.g., 0.1.0)')
    parser.add_argument('--min-pi-version', default='1.8.9', help='Minimum PixInsight version')
    parser.add_argument('--max-pi-version', default='1.9.99', help='Maximum PixInsight version')
    parser.add_argument('--platform', choices=['linux', 'macosx', 'windows', 'all'], 
                       default='all', help='Target platform(s)')
    parser.add_argument('--repo-root', default=None, help='Repository root path')
    
    args = parser.parse_args()
    
    # Determine repository root
    if args.repo_root:
        repo_root = Path(args.repo_root).resolve()
    else:
        # Script is in .github/scripts/
        script_dir = Path(__file__).parent
        repo_root = script_dir.parent.parent
    
    print("======================================================================")
    print(f"{MODULE_TITLE} Release Packager")
    print("======================================================================")
    print(f"Repository root: {repo_root}")
    print(f"Version: {args.version}")
    print(f"PixInsight version range: {args.min_pi_version}:{args.max_pi_version}")
    
    # Create dist directory
    dist_dir = repo_root / "dist"
    dist_dir.mkdir(exist_ok=True)
    
    # Determine platforms
    platforms = ['linux', 'macosx', 'windows'] if args.platform == 'all' else [args.platform]
    
    # Create packages for each platform
    packages = []
    for platform in platforms:
        pkg = create_package(platform, args.version, repo_root, dist_dir)
        if pkg:
            packages.append(pkg)
        else:
            print(f"Error: Failed to create package for {platform}")
            sys.exit(1)
    
    if not packages:
        print("Error: No packages created")
        sys.exit(1)
    
    # Generate updates.xri
    xri_path = generate_updates_xri(packages, args.version, args.min_pi_version, 
                                    args.max_pi_version, repo_root, dist_dir)
    
    print("\n======================================================================")
    print("✓ Release packaging completed successfully!")
    print("======================================================================")
    print(f"\nCreated {len(packages)} package(s):")
    for pkg in packages:
        print(f"  • {pkg['filename']}")
    print(f"\nDistribution directory: {dist_dir}")
    print("\nNext steps:")
    print("  1. Commit and push dist/ directory")
    print("  2. Create GitHub release")
    print("  3. Users can add repository URL to PixInsight")
    print("")

if __name__ == "__main__":
    main()
