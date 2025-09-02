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

#include <string>
#include <QSqlQuery>

QT_BEGIN_NAMESPACE
class QSqlDatabase;
QT_END_NAMESPACE

namespace sql_helpers {

void try_(QSqlQuery&, bool status);
QSqlQuery exec_query(QSqlDatabase&, const std::string& query_text);

void upgrade_schema_if_needed(QSqlDatabase&, int latest_schema_version, const std::string& schema_dir_path);

} // namespace sql_helpers
