#!/usr/bin/env python3
"""
PixInsight Module Build File Generator
Generates makefiles and Visual Studio project files for VeraLuxPixInsight module
"""

import os
import sys
import argparse
from pathlib import Path
from datetime import datetime
import uuid

# Module name
MODULE_NAME = "VeraLuxPixInsight"

def find_files(directory, extensions):
    """Recursively find files with given extensions"""
    files = []
    for ext in extensions:
        files.extend(Path(directory).rglob(f"*{ext}"))
    return sorted([f.relative_to(directory) for f in files])

def generate_unix_makefile_x64(platform, repo_root, src_files):
    """Generate makefile-x64 for Linux or macOS"""
    
    # Platform-specific settings
    if platform == "linux":
        compiler = "g++"
        platform_def = "-D__PCL_LINUX"
        extra_flags = "-D__PCL_AVX2 -D__PCL_FMA -mavx2 -mfma -fnon-call-exceptions"
        output_ext = "so"
        linker_flags = '-m64 -fPIC -pthread -Wl,-fuse-ld=gold -Wl,--enable-new-dtags -Wl,-z,noexecstack -Wl,-O1 -Wl,--gc-sections -s -shared -L"$(PCLLIBDIR64)" -L"$(PCLBINDIR64)/lib"'
        linker_libs = "-lpthread -lPCL-pxi -llz4-pxi -lzstd-pxi -lzlib-pxi -lRFC6234-pxi -llcms-pxi -lcminpack-pxi"
        obj_dir = f"{repo_root}/{platform}/g++/x64/Release"
        bin_dir = "linux"
    else:  # macosx
        compiler = "clang++"
        platform_def = "-D__PCL_MACOSX"
        extra_flags = "-msse4.2"
        output_ext = "dylib"
        linker_flags = '-arch x86_64 -fPIC -headerpad_max_install_names -Wl,-syslibroot,/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -mmacosx-version-min=12 -stdlib=libc++ -Wl,-dead_strip -dynamiclib -install_name @executable_path/VeraLuxPixInsight-pxm.dylib -L"$(PCLLIBDIR64)"'
        linker_libs = "-framework AppKit -lpthread -lPCL-pxi -llz4-pxi -lzstd-pxi -lzlib-pxi -lRFC6234-pxi -llcms-pxi -lcminpack-pxi"
        obj_dir = f"{repo_root}/{platform}/g++/x64/Release"
        bin_dir = "macosx"
    
    # Compiler flags (match PixInsight generator output ordering)
    if platform == "linux":
        compiler_flags = (
            '-c -pipe -pthread -m64 -fPIC -D_REENTRANT -D__PCL_LINUX '
            '-D__PCL_AVX2 -D__PCL_FMA -I"$(PCLINCDIR)" -I"$(PCLSRCDIR)/3rdparty" '
            '-mavx2 -mfma -minline-all-stringops -O3 -ffunction-sections -fdata-sections '
            '-ffast-math -fvisibility=hidden -fvisibility-inlines-hidden -fnon-call-exceptions '
            '-std=c++17 -Wall -Wno-parentheses'
        )
    else:  # macosx
        compiler_flags = (
            '-c -pipe -pthread -arch x86_64 -fPIC -isysroot '
            '/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk '
            '-mmacosx-version-min=12 -D_REENTRANT -D__PCL_MACOSX '
            '-I"$(PCLINCDIR)" -I"$(PCLSRCDIR)/3rdparty" -msse4.2 '
            '-minline-all-stringops -O3 -ffunction-sections -fdata-sections -ffast-math '
            '-fvisibility=hidden -fvisibility-inlines-hidden -std=c++17 -stdlib=libc++ '
            '-Wall -Wno-parentheses -Wno-extern-c-compat'
        )
    
    # Generate source file list
    src_list = " \\\n".join([f"../../src/{f}" for f in src_files])
    
    # Generate object file list
    obj_list = " \\\n".join([f"./x64/Release/src/{Path(f).with_suffix('.o')}" for f in src_files])
    
    # Generate dependency file list
    dep_list = " \\\n".join([f"./x64/Release/src/{Path(f).with_suffix('.d')}" for f in src_files])
    
    # Get unique subdirectories for pattern rules
    subdirs = set()
    for f in src_files:
        parent = Path(f).parent
        if str(parent) != '.':
            subdirs.add(str(parent))
    
    # Generate pattern rules for each subdirectory
    pattern_rules = []
    for subdir in sorted(subdirs):
        rule = f"""./x64/Release/src/{subdir}/%.o: ../../src/{subdir}/%.cpp
\tmkdir -p $(@D)
\t{compiler} {compiler_flags} -MMD -MP -MF"$(@:%.o=%.d)" -o"$@" "$<"
\t@echo ' '"""
        pattern_rules.append(rule)
    
    # Pattern rule for root directory
    root_rule = f"""./x64/Release/src/%.o: ../../src/%.cpp
\tmkdir -p $(@D)
\t{compiler} {compiler_flags} -MMD -MP -MF"$(@:%.o=%.d)" -o"$@" "$<"
\t@echo ' '"""
    pattern_rules.insert(0, root_rule)
    
    # Generate makefile content
    now = datetime.now().isoformat() + 'Z'
    platform_label = "MacOSX" if platform == "macosx" else "Linux"
    content = f"""######################################################################
# PixInsight Makefile Generator Script v1.144
# Copyright (C) 2009-2026 Pleiades Astrophoto
######################################################################
# Generated on .... {now}
# Project id ...... {MODULE_NAME}
# Project type .... Module
# Platform ........ {platform_label}/g++
# Configuration ... Release/x64
######################################################################

OBJ_DIR="{obj_dir}"

.PHONY: all
all: $(OBJ_DIR)/{MODULE_NAME}-pxm.{output_ext}

#
# Source files
#

SRC_FILES= \\
{src_list}

#
# Object files
#

OBJ_FILES= \\
{obj_list}

#
# Dependency files
#

DEP_FILES= \\
{dep_list}

#
# Rules
#

-include $(DEP_FILES)

$(OBJ_DIR)/{MODULE_NAME}-pxm.{output_ext}: $(OBJ_FILES)
\tmkdir -p $(OBJ_DIR)
\t{compiler} {linker_flags} -o $(OBJ_DIR)/{MODULE_NAME}-pxm.{output_ext} $(OBJ_FILES) {linker_libs}
\t$(MAKE) -f ./makefile-x64 --no-print-directory post-build

.PHONY: clean
clean:
\trm -f $(OBJ_FILES) $(DEP_FILES) $(OBJ_DIR)/{MODULE_NAME}-pxm.{output_ext}

.PHONY: post-build
post-build:
\tcp $(OBJ_DIR)/{MODULE_NAME}-pxm.{output_ext} $(PCLBINDIR64)
\tmkdir -p ../../bin/{bin_dir}
\tcp $(OBJ_DIR)/{MODULE_NAME}-pxm.{output_ext} ../../bin/{bin_dir}/

{chr(10).join(pattern_rules)}

"""
    return content

def generate_unix_makefile(platform):
    """Generate main Makefile wrapper"""
    now = datetime.now().isoformat() + 'Z'
    platform_label = "MacOSX" if platform == "macosx" else "Linux"
    content = f"""######################################################################
# PixInsight Makefile Generator Script v1.144
# Copyright (C) 2009-2026 Pleiades Astrophoto
######################################################################
# Generated on .... {now}
# Project id ...... {MODULE_NAME}
# Project type .... Module
# Platform ........ {platform_label}/g++
# Configuration ... Release/all
######################################################################

#
# Targets
#

.PHONY: all
all: 
\t$(MAKE) -f ./makefile-x64 --no-print-directory

.PHONY: clean
clean:
\t$(MAKE) -f ./makefile-x64 --no-print-directory clean

"""
    return content

def generate_vcxproj(repo_root, src_files, header_files, resource_files):
    """Generate Visual Studio project file"""
    now = datetime.now().isoformat() + 'Z'
    
    # Generate ClCompile items
    compile_items = "\n".join([f'    <ClCompile Include="..\\..\\src\\{str(f).replace("/", "\\\\")}\"/>' 
                               for f in src_files])
    
    # Generate ClInclude items
    include_items = "\n".join([f'    <ClInclude Include="..\\..\\src\\{str(f).replace("/", "\\\\")}\"/>' 
                               for f in header_files])
    
    # Generate None (resource) items
    resource_items = "\n".join([f'    <None Include="..\\..\\{str(f).replace("/", "\\\\")}\"/>' 
                                for f in resource_files])
    
    content = f"""<?xml version="1.0" encoding="utf-8"?>
<!--
######################################################################
# PixInsight Makefile Generator Script v1.144
# Copyright (C) 2009-2026 Pleiades Astrophoto
######################################################################
# Generated on .... {now}
# Project id ...... {MODULE_NAME}
# Project type .... Module
# Platform ........ Windows/vc17
######################################################################
-->
<Project DefaultTargets="Build" ToolsVersion="17.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <ProjectGuid>{{{str(uuid.uuid4()).upper()}}}</ProjectGuid>
    <RootNamespace>PixInsight</RootNamespace>
    <Keyword>Win32Proj</Keyword>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>DynamicLibrary</ConfigurationType>
    <CharacterSet>Unicode</CharacterSet>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <PlatformToolset>v143</PlatformToolset>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>DynamicLibrary</ConfigurationType>
    <CharacterSet>Unicode</CharacterSet>
    <PlatformToolset>v143</PlatformToolset>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings">
  </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="PropertySheets">
    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="PropertySheets">
    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <_ProjectFileVersion>10.0.40219.1</_ProjectFileVersion>
    <OutDir Condition="'$(Configuration)|$(Platform)'=='Release|x64'">$(PCLBINDIR64)\\</OutDir>
    <IntDir Condition="'$(Configuration)|$(Platform)'=='Release|x64'">$(Platform)\\$(Configuration)\\</IntDir>
    <OutDir Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">$(PCLBINDIR64)\\</OutDir>
    <IntDir Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">$(Platform)\\$(Configuration)\\</IntDir>
    <TargetName Condition="'$(Configuration)|$(Platform)'=='Release|x64'">{MODULE_NAME}-pxm</TargetName>
    <TargetName Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">{MODULE_NAME}-pxm</TargetName>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Midl>
      <TargetEnvironment>X64</TargetEnvironment>
    </Midl>
    <ClCompile>
      <Optimization>MaxSpeed</Optimization>
      <InlineFunctionExpansion>AnySuitable</InlineFunctionExpansion>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <FavorSizeOrSpeed>Speed</FavorSizeOrSpeed>
      <OmitFramePointers>true</OmitFramePointers>
      <WholeProgramOptimization>true</WholeProgramOptimization>
      <AdditionalIncludeDirectories>$(PCLINCDIR);$(PCLSRCDIR)\\3rdparty;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>WIN32;WIN64;_WINDOWS;UNICODE;__PCL_WINDOWS;__PCL_NO_WIN32_MINIMUM_VERSIONS;__PCL_AVX2;__PCL_FMA;_NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <StringPooling>true</StringPooling>
      <ExceptionHandling>Async</ExceptionHandling>
      <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>
      <BufferSecurityCheck>false</BufferSecurityCheck>
      <FunctionLevelLinking>false</FunctionLevelLinking>
      <FloatingPointModel>Fast</FloatingPointModel>
      <PrecompiledHeader></PrecompiledHeader>
      <WarningLevel>Level3</WarningLevel>
      <DebugInformationFormat></DebugInformationFormat>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <RuntimeTypeInfo>true</RuntimeTypeInfo>
      <EnableEnhancedInstructionSet>AdvancedVectorExtensions2</EnableEnhancedInstructionSet>
      <LanguageStandard>stdcpp17</LanguageStandard>
      <AdditionalOptions>/Zc:__cplusplus /permissive- %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <AdditionalDependencies>PCL-pxi.lib;lz4-pxi.lib;zstd-pxi.lib;zlib-pxi.lib;RFC6234-pxi.lib;lcms-pxi.lib;cminpack-pxi.lib;kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;userenv.lib;imm32.lib;shlwapi.lib;ws2_32.lib;wldap32.lib;mscms.lib;winmm.lib;crypt32.lib;normaliz.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <OutputFile>$(PCLBINDIR64)\\{MODULE_NAME}-pxm.dll</OutputFile>
      <AdditionalLibraryDirectories>$(PCLLIBDIR64);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <ModuleDefinitionFile></ModuleDefinitionFile>
      <DelayLoadDLLs>%(DelayLoadDLLs)</DelayLoadDLLs>
      <SubSystem>Windows</SubSystem>
      <LargeAddressAware>true</LargeAddressAware>
      <LinkTimeCodeGeneration>UseLinkTimeCodeGeneration</LinkTimeCodeGeneration>
      <SupportUnloadOfDelayLoadedDLL>true</SupportUnloadOfDelayLoadedDLL>
      <ImportLibrary>$(Platform)\\$(Configuration)\\$(ProjectName).lib</ImportLibrary>
      <TargetMachine>MachineX64</TargetMachine>
      <GenerateDebugInformation>false</GenerateDebugInformation>
    </Link>
    <PostBuildEvent>
      <Command>if not exist "..\\..\\bin\\windows" mkdir "..\\..\\bin\\windows"
xcopy /Y "$(OutDir){MODULE_NAME}-pxm.dll" "..\\..\\bin\\windows\\"</Command>
    </PostBuildEvent>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Midl>
      <TargetEnvironment>X64</TargetEnvironment>
    </Midl>
    <ClCompile>
      <Optimization>Disabled</Optimization>
      <AdditionalIncludeDirectories>$(PCLINCDIR);$(PCLSRCDIR)\\3rdparty;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>WIN32;WIN64;_WINDOWS;UNICODE;__PCL_WINDOWS;__PCL_NO_WIN32_MINIMUM_VERSIONS;__PCL_AVX2;__PCL_FMA;_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ExceptionHandling>Async</ExceptionHandling>
      <MinimalRebuild>true</MinimalRebuild>
      <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>
      <RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>
      <PrecompiledHeader></PrecompiledHeader>
      <WarningLevel>Level3</WarningLevel>
      <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <RuntimeTypeInfo>true</RuntimeTypeInfo>
      <EnableEnhancedInstructionSet>AdvancedVectorExtensions2</EnableEnhancedInstructionSet>
      <LanguageStandard>stdcpp17</LanguageStandard>
      <AdditionalOptions>/Zc:__cplusplus /permissive- %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <AdditionalDependencies>PCL-pxi.lib;lz4-pxi.lib;zstd-pxi.lib;zlib-pxi.lib;RFC6234-pxi.lib;lcms-pxi.lib;cminpack-pxi.lib;kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;userenv.lib;imm32.lib;shlwapi.lib;ws2_32.lib;wldap32.lib;mscms.lib;winmm.lib;crypt32.lib;normaliz.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <OutputFile>$(PCLBINDIR64)\\{MODULE_NAME}-pxm.dll</OutputFile>
      <AdditionalLibraryDirectories>$(PCLLIBDIR64);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <ModuleDefinitionFile></ModuleDefinitionFile>
      <SubSystem>Windows</SubSystem>
      <LargeAddressAware>true</LargeAddressAware>
      <ImportLibrary>$(Platform)\\$(Configuration)\\$(ProjectName).lib</ImportLibrary>
      <TargetMachine>MachineX64</TargetMachine>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
    <PostBuildEvent>
      <Command>if not exist "..\\..\\bin\\windows" mkdir "..\\..\\bin\\windows"
xcopy /Y "$(OutDir){MODULE_NAME}-pxm.dll" "..\\..\\bin\\windows\\"</Command>
    </PostBuildEvent>
  </ItemDefinitionGroup>
  <ItemGroup>
{compile_items}
  </ItemGroup>
  <ItemGroup>
{include_items}
  </ItemGroup>
  <ItemGroup>
{resource_items}
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets"/>
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>
"""
    return content

def generate_vcxproj_filters(src_files, header_files, resource_files):
    """Generate Visual Studio filters file"""
    now = datetime.now().isoformat() + 'Z'
    
    # Generate ClCompile items with filters
    compile_items = "\n".join([f'    <ClCompile Include="..\\..\\src\\{str(f).replace("/", "\\\\")}">\\n        <Filter>Source Files</Filter>\\n    </ClCompile>' 
                               for f in src_files])
    
    # Generate ClInclude items with filters
    include_items = "\n".join([f'    <ClInclude Include="..\\..\\src\\{str(f).replace("/", "\\\\")}">\\n        <Filter>Header Files</Filter>\\n    </ClInclude>' 
                               for f in header_files])
    
    # Generate None items with filters
    resource_items = "\n".join([f'    <None Include="..\\..\\{str(f).replace("/", "\\\\")}">\\n        <Filter>Image Files</Filter>\\n    </None>' 
                                for f in resource_files])
    
    content = f"""<?xml version="1.0" encoding="utf-8"?>
<!--
######################################################################
# PixInsight Makefile Generator Script v1.144
# Copyright (C) 2009-2026 Pleiades Astrophoto
######################################################################
# Generated on .... {now}
# Project id ...... {MODULE_NAME}
# Project type .... Module
# Platform ........ Windows/vc17
######################################################################
-->
<Project DefaultTargets="Build" ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="Source Files">
      <UniqueIdentifier>{{{str(uuid.uuid4()).lower()}}}</UniqueIdentifier>
    </Filter>
    <Filter Include="Header Files">
      <UniqueIdentifier>{{{str(uuid.uuid4()).lower()}}}</UniqueIdentifier>
    </Filter>
    <Filter Include="Image Files">
      <UniqueIdentifier>{{{str(uuid.uuid4()).lower()}}}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
  <ItemGroup>
{compile_items}
  </ItemGroup>
  <ItemGroup>
{include_items}
  </ItemGroup>
  <ItemGroup>
{resource_items}
  </ItemGroup>
</Project>
"""
    return content

def main():
    parser = argparse.ArgumentParser(description='Generate build files for VeraLuxPixInsight module')
    parser.add_argument('--platform', choices=['linux', 'macosx', 'windows', 'all'], 
                       default='all', help='Target platform(s)')
    parser.add_argument('--repo-root', default=None, 
                       help='Repository root path (default: auto-detect)')
    
    args = parser.parse_args()
    
    # Determine repository root
    if args.repo_root:
        repo_root = Path(args.repo_root).resolve()
    else:
        # Assume script is in .github/scripts/ directory
        script_dir = Path(__file__).parent
        repo_root = script_dir.parent.parent
    
    print(f"Repository root: {repo_root}")
    
    # Find source files
    src_dir = repo_root / "src"
    if not src_dir.exists():
        print(f"Error: Source directory not found: {src_dir}")
        sys.exit(1)
    
    cpp_files = find_files(src_dir, [".cpp"])
    h_files = find_files(src_dir, [".h"])
    svg_files = find_files(repo_root, [".svg"])
    
    print(f"Found {len(cpp_files)} .cpp files")
    print(f"Found {len(h_files)} .h files")
    print(f"Found {len(svg_files)} .svg files")
    
    platforms = ['linux', 'macosx', 'windows'] if args.platform == 'all' else [args.platform]
    
    for platform in platforms:
        print(f"\nGenerating build files for {platform}...")
        
        if platform in ['linux', 'macosx']:
            # Generate Unix makefiles
            platform_dir = repo_root / platform / "g++"
            platform_dir.mkdir(parents=True, exist_ok=True)
            
            # Generate main Makefile
            makefile_content = generate_unix_makefile(platform)
            makefile_path = platform_dir / "Makefile"
            makefile_path.write_text(makefile_content)
            print(f"  Created: {makefile_path}")
            
            # Generate makefile-x64
            makefile_x64_content = generate_unix_makefile_x64(platform, str(repo_root), cpp_files)
            makefile_x64_path = platform_dir / "makefile-x64"
            makefile_x64_path.write_text(makefile_x64_content)
            print(f"  Created: {makefile_x64_path}")
            
        elif platform == 'windows':
            # Generate Visual Studio project files
            platform_dir = repo_root / "windows" / "vc17"
            platform_dir.mkdir(parents=True, exist_ok=True)
            
            # Generate .vcxproj
            vcxproj_content = generate_vcxproj(str(repo_root), cpp_files, h_files, svg_files)
            vcxproj_path = platform_dir / f"{MODULE_NAME}.vcxproj"
            vcxproj_path.write_text(vcxproj_content)
            print(f"  Created: {vcxproj_path}")
            
            # Generate .vcxproj.filters
            filters_content = generate_vcxproj_filters(cpp_files, h_files, svg_files)
            filters_path = platform_dir / f"{MODULE_NAME}.vcxproj.filters"
            filters_path.write_text(filters_content)
            print(f"  Created: {filters_path}")
    
    print("\nBuild files generated successfully!")

if __name__ == "__main__":
    main()
