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
#include "spdx_parser.hpp"
#include <ostream>
#include <stdexcept>
#include <QFile>

using namespace Qt::StringLiterals;

namespace spdx {

static
Relationship parse_relationship(QString relationship_text)
{
    auto parts = relationship_text.split(' ');
    if(parts.size() != 3) {
        throw std::runtime_error(u"Unexpected relationship: %1"_s.arg(relationship_text).toStdString());
    }
    return {parts[0], parts[1], parts[2]};
}

SPDX_Version Parser::parse_version()
{
    line = spdx_file->readLine();
    if(!line.startsWith(u"SPDXVersion:"_s)) {
        throw std::runtime_error(u"Error: missing SPDXVersion field at beginning of %1"_s.arg(spdx_file->fileName()).toStdString());
    }
    auto version_str = line.section('-', 1);
    auto version_parts = version_str.split('.');
    if(version_parts.size() < 2) {
        throw std::runtime_error(u"Error: unexpected format for SPDXVersion field at beginning of %1"_s.arg(spdx_file->fileName()).toStdString());
    }
    bool ok;
    auto major_version = version_parts[0].toInt(&ok);
    if(!ok) {
        throw std::runtime_error(u"Error: unexpected format for SPDXVersion field at beginning of %1"_s.arg(spdx_file->fileName()).toStdString());
    }
    auto minor_version = version_parts[1].toInt(&ok);
    if(!ok) {
        throw std::runtime_error(u"Error: unexpected format for SPDXVersion field at beginning of %1"_s.arg(spdx_file->fileName()).toStdString());
    }
    return {major_version, minor_version};
}

Package Parser::parse_package()
{
    temp_relationships.clear();
    Package package;
    line = spdx_file->readLine();
    bool done = false;
    while(!spdx_file->atEnd() && !done) {
        if(line.startsWith(u"SPDXID:"_s)) {
            package.spdx_id = parse_single_line_field();
        } else if(line.startsWith(u"PackageName:"_s)) {
            package.name = parse_single_line_field();
        } else if(line.startsWith(u"PackageVersion:"_s)) {
            package.version = parse_single_line_field();
        } else if(line.startsWith(u"PackageCopyrightText:"_s)) {
            package.copyright = parse_multiline_text_field();
        } else if(line.startsWith(u"PackageLicenseConcluded:"_s)) {
            package.license = parse_single_line_field(); // TODO: <SPDX License Expression>
        } else if(line.startsWith(u"Relationship:"_s)) {
            temp_relationships.push_back(parse_single_line_field());
        } else if(line.trimmed().isEmpty()) {
            // Reached end of definition
            if(package.name.isEmpty()) {
                // Not a package definition - continue
                package = {};
                line = spdx_file->readLine();
            } else {
                // Lazily parse the relationship field to avoid having to support
                // relationships found in elements that aren't package definitions
                for(const auto& relationship : temp_relationships) {
                    package.relationships.push_back(parse_relationship(relationship));
                }
                done = true;
            }
            temp_relationships.clear();
        } else {
            line = spdx_file->readLine();
        }
    }
    return package;
}

QString Parser::parse_single_line_field()
{
    auto field = line.section(u": "_s, 1).trimmed();
    line = spdx_file->readLine();
    return field;
}

QString Parser::parse_multiline_text_field()
{
    auto field = parse_single_line_field();
    if(field.startsWith(u"<text>"_s)) {
        field = field.sliced(6);
        field.append('\n');
        while(!spdx_file->atEnd() && !field.endsWith(u"</text>\n"_s)) {
            field.append(line.trimmed()).append('\n');
            line = spdx_file->readLine();
        }
        field = field.sliced(0, field.size() - 8);
    }
    return field;
}

std::ostream& operator<<(std::ostream& out, const spdx::Package& package)
{
    out << package.name.toStdString() << ":\n"
           "  SPDX ID: " << package.spdx_id.toStdString() << "\n"
           "  Version: " << package.version.toStdString() << "\n"
           "  Copyright: " << package.copyright.toStdString() << "\n"
           "  License: " << package.license.toStdString() << "\n";
    if(!package.relationships.empty()) {
        out << "  Relationships:\n";
        for(const auto& relationship : package.relationships) {
            out << "    ";
            if(relationship.spdx_id1 == package.spdx_id) {
                out << "this";
            } else {
                out << relationship.spdx_id1.toStdString();
            }
            out << " " << relationship.kind.toStdString() << " ";
            if(relationship.spdx_id2 == package.spdx_id) {
                out << "this";
            } else {
                out << relationship.spdx_id2.toStdString();
            }
            out << "\n";
        }
    }
    return out;
}

} // namespace spdx
