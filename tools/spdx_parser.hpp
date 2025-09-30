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

#pragma once

#include <iosfwd>
#include <vector>
#include <QString>

QT_BEGIN_NAMESPACE
class QFile;
QT_END_NAMESPACE

namespace spdx {

constexpr int major_version = 2;
constexpr int min_supported_minor_version = 3;

struct SPDX_Version {
    int major;
    int minor;
};

struct Relationship {
    QString spdx_id1;
    QString kind;
    QString spdx_id2;
};

struct Package {
    QString spdx_id;
    QString name;
    QString version;
    QString copyright;
    QString license;
    std::vector<Relationship> relationships;
};
std::ostream& operator<<(std::ostream&, const spdx::Package&);

struct Parser {
    explicit
    Parser(QFile& spdx_file) : spdx_file(&spdx_file) {}

    SPDX_Version parse_version();
    Package parse_package();
private:
    QString parse_single_line_field();
    QString parse_multiline_text_field();

    QFile* spdx_file;
    QString line;
    std::vector<QString> temp_relationships;
};

} // namespace spdx
