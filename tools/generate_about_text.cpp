/*
Accountant - accounting program
Copyright (C) 2025  Cole Blakley

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <iostream>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QString>
#ifdef SPDX_PROVIDED
#include <algorithm>
#include <span>
#include <utility>
#include <vector>
#include "spdx_parser.hpp"
#endif /* SPDX_PROVIDED */

using namespace Qt::StringLiterals;

#ifdef SPDX_PROVIDED
struct PackageInfo {
    spdx::Package package;
    std::vector<QString> direct_dependencies;
};

using StringPair = std::pair<QString, QString>;


// The SPDX file doesn't provide good human-readable names for these packages, so use these names instead
static const StringPair pretty_names[] = {
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-blake2"_s,                     u"BLAKE2 (reference implementation)"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-easing"_s,                     u"Easing Equations by Robert Penner"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-forkfd"_s,                     u"forkfd"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-md4"_s,                        u"MD4"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-md5"_s,                        u"MD5"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-qeventdispatcher-cf"_s,        u"QEventDispatcher on macOS"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-rfc6234"_s,                    u"Secure Hash Algorithms SHA-384 and SHA-512"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-sha1"_s,                       u"Secure Hash Algorithm SHA-1"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-sha3-endian"_s,                u"Secure Hash Algorithm SHA-3 - brg_endian"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-sha3-keccak"_s,                u"Secure Hash Algorithm SHA-3 - Keccak"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-siphash"_s,                    u"SipHash Algorithm"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-tika-mimetypes"_s,             u"Apache Tika MimeType Definitions"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-tinycbor"_s,                   u"TinyCBOR"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-unicode-character-database"_s, u"Unicode Character Database (UCD)"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-unicode-cldr"_s,               u"Unicode Common Locale Data Repository (CLDR)"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-DBus-Attribution-libdbus-1-headers"_s,          u"libdus-1 headers"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-aglfn"_s,                       u"Adobe Glyph List For New Fonts"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-grayraster"_s,                  u"Anti-aliasing rasterizer from FreeType 2"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-icc-srgb-color-profile"_s,      u"sRGB color profile icc file"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-opengl-es2-headers"_s,          u"OpenGL ES 2 Headers"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-opengl-headers"_s,              u"OpenGL Headers"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-pixman"_s,                      u"Pixman"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-rhi-miniengine-d3d12-mipmap"_s, u"Mipmap generator for D3D12"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-smooth-scaling-algorithm"_s,    u"Smooth Scaling Algorithm"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-vulkan-xml-spec"_s,             u"Vulkan API Registry"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-vulkanmemoryallocator"_s,       u"Vulkan Memory Allocator"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-webgradients"_s,                u"WebGradients"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-xserverhelper"_s,               u"X Server helper"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Platform-Attribution-extra-cmake-modules"_s,    u"extra-cmake-modules"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Platform-Attribution-kwin"_s,                   u"KWin"_s},
    {u"SPDXRef-Package-qtbase-qt-module-Core"_s,                                                  u"Qt Core"_s},
    {u"SPDXRef-Package-qtbase-qt-module-DBus"_s,                                                  u"Qt DBus"_s},
    {u"SPDXRef-Package-qtbase-qt-module-Gui"_s,                                                   u"Qt Gui"_s},
    {u"SPDXRef-Package-qtbase-qt-module-Sql"_s,                                                   u"Qt Sql"_s},
    {u"SPDXRef-Package-qtbase-qt-module-Widgets"_s,                                               u"Qt Widgets"_s},
    {u"SPDXRef-Package-qtbase-qt-module-Platform"_s,                                              u"Qt Platform"_s},
    {u"SPDXRef-Package-qtbase-qt-module-PlatformModuleInternal"_s,                                u"Qt PlatformModuleInternal"_s},
    {u"SPDXRef-Package-qtbase-qt-module-PlatformPluginInternal"_s,                                u"Qt PlatformPluginInternal"_s},
};

// The SPDX file doesn't provide good human-readable names for these licenses, so use these names instead
static const StringPair pretty_licenses[] = {
    {u"LicenseRef-SHA1-Public-Domain"_s, u"Public Domain"_s},
    {u"LicenseRef-ICC-License"_s,        u"ICC"_s},
};

// The SPDX file is missing some copyright information, so we provide it here
static const StringPair additional_copyrights[] = {
    {u"SPDXRef-Package-qtbase-system-3rdparty-ICU"_s,     u"Copyright © 2016-2025 Unicode, Inc."_s},
    {u"SPDXRef-Package-qtbase-system-3rdparty-Libb2"_s,   u"Public Domain"_s},
    {u"SPDXRef-Package-qtbase-system-3rdparty-SQLite3"_s, u"Public Domain"_s}
};

// The SPDX file provides a choice of licenses for these packages - this is the license we will use for them
// for this project
static const StringPair license_choices[] = {
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Core-Attribution-blake2"_s,            u"Apache-2.0"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-DBus-Attribution-libdbus-1-headers"_s, u"GPL-2.0-or-later"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-grayraster"_s,         u"FTL"_s},
    {u"SPDXRef-Package-qtbase-qt-3rdparty-sources-Gui-Attribution-vulkan-xml-spec"_s,    u"MIT"_s},
    {u"SPDXRef-Package-qtbase-qt-module-Core"_s,                                         u"GPL-3.0-only"_s},
    {u"SPDXRef-Package-qtbase-qt-module-DBus"_s,                                         u"GPL-3.0-only"_s},
    {u"SPDXRef-Package-qtbase-qt-module-GlobalConfigPrivate"_s,                          u"GPL-3.0-only"_s},
    {u"SPDXRef-Package-qtbase-qt-module-Gui"_s,                                          u"GPL-3.0-only"_s},
    {u"SPDXRef-Package-qtbase-qt-module-Platform"_s,                                     u"GPL-3.0-only"_s},
    {u"SPDXRef-Package-qtbase-qt-module-PlatformModuleInternal"_s,                       u"GPL-3.0-only"_s},
    {u"SPDXRef-Package-qtbase-qt-module-PlatformPluginInternal"_s,                       u"GPL-3.0-only"_s},
    {u"SPDXRef-Package-qtbase-qt-module-Sql"_s,                                          u"GPL-3.0-only"_s},
    {u"SPDXRef-Package-qtbase-qt-module-Widgets"_s,                                      u"GPL-3.0-only"_s},
    {u"SPDXRef-Package-qtbase-qt-plugin-QSQLiteDriverPlugin"_s,                          u"GPL-3.0-only"_s},
    {u"SPDXRef-Package-qtbase-system-3rdparty-ICU"_s,                                    u"Unicode-3.0"_s},
    {u"SPDXRef-Package-qtbase-system-3rdparty-SQLite3"_s,                                u"Public Domain"_s}
};

// The direct dependencies of this application
static const QString app_dependencies[] = {
    u"SPDXRef-Package-qtbase-qt-module-Core"_s,
    u"SPDXRef-Package-qtbase-qt-module-Sql"_s,
    u"SPDXRef-Package-qtbase-qt-module-Widgets"_s,
    u"SPDXRef-Package-qtbase-qt-plugin-QSQLiteDriverPlugin"_s
};

static
bool load_from_spdx(const char* spdx_path, QHash<QString, PackageInfo>& package_info, std::vector<QString>& transitive_dependencies);

static
void write_packages_to_markdown(const QHash<QString, PackageInfo>&, std::span<const QString> transitive_dependencies, QFile& output_file);

static
bool version_check(spdx::SPDX_Version, std::string filename);

static
void gather_dependencies(const QHash<QString, PackageInfo>& package_info,
                         std::vector<QString>& transitive_dependencies,
                         const QString& package_id);

static
bool fixup_package(spdx::Package&);
#endif /* SPDX_PROVIDED */

int main(int argc, char **argv)
{
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " spdx_path output_file\n";
        return 1;
    }
    #ifdef SPDX_PROVIDED
    QHash<QString, PackageInfo> package_info;
    std::vector<QString> transitive_dependencies;
    if(!load_from_spdx(argv[1], package_info, transitive_dependencies)) {
        return 1;
    }
    #endif /* SPDX_PROVIDED */

    QFile output_file{argv[2]};
    if(!output_file.open(QFile::WriteOnly)) {
        std::cerr << "Error: could not open output file: '"
                  << output_file.fileName().toStdString() << "'\n";
        return 1;
    }

    output_file.write(
        "**Accountant** - accounting program\n\n"
        "Copyright (C) 2025 Cole Blakley\n\n"
        "This program is licensed under the [GNU General Public License, version "
        "3](https://www.gnu.org/licenses/gpl-3.0.html).\n\n"
        "----\n\n"
        "This program uses the following third-party libraries:\n");
    #ifdef SPDX_PROVIDED
    write_packages_to_markdown(package_info, transitive_dependencies, output_file);
    #else
    // We don't have an SBOM, so make a best effort at including the libraries used
    output_file.write(
        "- Qt Core (software version " QT_VERSION_STR ", GPLv3 license)\n"
        "- Qt SQL (software version " QT_VERSION_STR ", GPLv3 license)\n"
        "- Qt Widgets (software version " QT_VERSION_STR ", GPLv3 license)\n"
        "- SQLite (software version 3, Public Domain)\n\n"
        "For more information about Qt licensing and accessing Qt source code, visit https://www.qt.io/qt-licensing.");
    #endif /* SPDX_PROVIDED */

    return 0;
}

#ifdef SPDX_PROVIDED
// Read from an SPDX file and gather metadata for each package (in package_info with the SPDX ID as the key) and gather
// a list of all transitive dependencies of the declared app_dependencies
static
bool load_from_spdx(const char* spdx_path, QHash<QString, PackageInfo>& package_info, std::vector<QString>& transitive_dependencies)
{
    QFile spdx_file{spdx_path};
    if(!spdx_file.open(QFile::ReadOnly | QFile::Text)) {
        std::cerr << "Error: failed to open SPDX file: '" << spdx_file.fileName().toStdString() <<"'\n";
        return false;
    }

    spdx::Parser parser{spdx_file};
    auto spdx_version = parser.parse_version();
    if(!version_check(spdx_version, spdx_file.fileName().toStdString())) {
        return false;
    }

    while(!spdx_file.atEnd()) {
        auto package = parser.parse_package();
        if(package.name.isEmpty()) {
            continue;
        }
        auto p = package_info.insert(package.spdx_id, PackageInfo{package, {}});
        for(const auto& relationship : p->package.relationships) {
            if(relationship.kind == "DEPENDS_ON") {
                if(p->package.spdx_id == relationship.spdx_id1) {
                    p->direct_dependencies.push_back(relationship.spdx_id2);
                } else {
                    p->direct_dependencies.push_back(relationship.spdx_id1);
                }
            }
        }
    }

    for(const QString& package_id : app_dependencies) {
        gather_dependencies(package_info, transitive_dependencies, package_id);
    }

    bool success = true;
    for(const QString& package_id : transitive_dependencies) {
        auto it = package_info.find(package_id);
        success |= fixup_package(it->package);
    }
    std::ranges::sort(transitive_dependencies, std::less<QString>{},
                      [&](const QString& id) { return package_info.find(id)->package.name; });
    return success;
}

// Write selected metadata for each of the list of transitive_dependencies to a Markdown document
static
void write_packages_to_markdown(const QHash<QString, PackageInfo>& package_info, std::span<const QString> transitive_dependencies, QFile& output_file)
{
    for(const QString& package_id : transitive_dependencies) {
        auto it = package_info.find(package_id);
        auto& package = it->package;
        output_file.write("\n**");
        output_file.write(package.name.toUtf8());
        output_file.write("**");
        if(package.version != u"unknown"_s) {
            output_file.write(" (Version ");
            output_file.write(package.version.toUtf8());
            output_file.write(")");
        }
        output_file.write("\n- ");
        output_file.write(package.copyright.toUtf8());
        output_file.write("\n- License: ");
        output_file.write(package.license.toUtf8());
        output_file.write("\n");
    }
}

// Check that we know how to parse the SPDX file
static
bool version_check(spdx::SPDX_Version spdx_version, std::string filename)
{
    if(spdx_version.major > spdx::major_version) {
        std::cerr << "Error: " << filename << " has higher major SPDX version (" << spdx_version.major << ") "
                     "than the SPDX processor supports (" << spdx::major_version << ")\n";
        return false;
    }

    if(spdx_version.major < spdx::major_version) {
        std::cerr << "Warning: " << filename << " has lower major SPDX version (" << spdx_version.major << ") "
                     "than the SPDX processor supports (" << spdx::major_version << ")\n";
    }

    if(spdx_version.minor < spdx::min_supported_minor_version) {
        std::cerr << "Warning: " << filename << " has lower minor SPDX version (" << spdx_version.minor << ") "
                     "than the SPDX processor supports (" << spdx::min_supported_minor_version << ")\n";
    }
    return true;
}

// Build a map of SPDX ID to package metadata, as well as a sorted list of transitive dependencies of package_id
static
void gather_dependencies(const QHash<QString, PackageInfo>& package_info,
                         std::vector<QString>& transitive_dependencies,
                         const QString& package_id)
{
    {
        auto it = std::ranges::lower_bound(transitive_dependencies, package_id);
        if(it != transitive_dependencies.end() && *it == package_id) {
            // package_id and its dependencies are already present
            return;
        }
        transitive_dependencies.insert(it, package_id);
    }
    auto it = package_info.find(package_id);
    if(it != package_info.end()) {
        for(const QString& dependency_id : it->direct_dependencies) {
            gather_dependencies(package_info, transitive_dependencies, dependency_id);
        }
    }
}

// Format, cleanup, and validate the package metadata
static
bool fixup_package(spdx::Package& package)
{
    bool success = true;
    // Fixup package name
    if(auto it = std::ranges::find(pretty_names, package.spdx_id, &StringPair::first); it != std::ranges::end(pretty_names)) {
        package.name = it->second;
    }
    if(package.name.isEmpty()) {
        success = false;
        std::cerr << "Error: package '" << package.spdx_id.toStdString() << "' has an empty package name\n";
    } else {
        package.name[0] = package.name[0].toTitleCase();
    }

    // Fixup copyright line
    if(package.copyright == u"NOASSERTION"_s) {
        auto it = std::ranges::find(additional_copyrights, package.spdx_id, &StringPair::first);
        if(it != std::ranges::end(additional_copyrights)) {
            package.copyright = it->second;
        }
    }

    // Fixup license name
    if(auto it = std::ranges::find(pretty_licenses, package.license, &StringPair::first); it != std::ranges::end(pretty_licenses)) {
        package.license = it->second;
    }

    // Select a license if there are multiple available
    if(auto choice = std::ranges::find(license_choices, package.spdx_id, &StringPair::first); choice != std::ranges::end(license_choices)) {
        auto choices = package.license.split(' ');
        if(std::ranges::find(choices, choice->second) == choices.end() && package.license != u"NOASSERTION"_s) {
            success = false;
            std::cerr << "Error: invalid license choice for package '" << package.name.toStdString() << "': "
                         "'" << choice->second.toStdString() << "' is not among the available licenses for this package\n";
        } else {
            package.license = choice->second;
        }
    }

    // Check that if there are multiple license choices, one is selected
    auto choices = package.license.split(' ');
    if(std::ranges::find(choices, u"OR"_s) != std::ranges::end(choices)) {
        success = false;
        std::cerr << "Error: package '" << package.name.toStdString() << "' has multiple license options and does not have a license chosen\n";
    }
    return success;
}
#endif /* SPDX_PROVIDED */
