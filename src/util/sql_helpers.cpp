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

#include "sql_helpers.hpp"
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>

namespace sql_helpers {

void try_(QSqlQuery& query, bool status)
{
    if(!status) {
        throw std::runtime_error(query.lastError().text().toStdString());
    }
}

QSqlQuery exec_query(QSqlDatabase& db, const std::string& query_text)
{
    QSqlQuery query{db};
    try_(query, query.exec(QString::fromStdString(query_text)));
    return query;
}

void upgrade_schema_if_needed(QSqlDatabase& db, int latest_schema_version, const std::string& schema_dir_path)
{
    std::filesystem::path schema_folder{schema_dir_path};
    if(!std::filesystem::exists(schema_dir_path)) {
        throw std::runtime_error("Schema folder path does not exist");
    }
    if(!std::filesystem::is_directory(schema_dir_path)) {
        throw std::runtime_error("Schema folder path is not a directory");
    }

    auto query = exec_query(db, "pragma user_version");
    try_(query, query.next());
    auto schema_version = query.value(0).toInt();
    if(schema_version > latest_schema_version) {
        throw std::runtime_error("Database has newer schema version than this software supports");
    } else if(schema_version < latest_schema_version) {
        // Migrate to the latest schema
        for(auto v = schema_version + 1; v <= latest_schema_version; ++v) {
            try {
                db.transaction();
                auto schema_path = schema_folder / std::format("{}-schema.sql", v);
                std::ifstream schema_file{schema_path};
                if(!schema_file) {
                    throw std::runtime_error(std::format("Schema migration failed - missing file: '{}'", schema_path.filename().string()));
                }
                std::noskipws(schema_file);
                std::string command;
                // Each command must be on a single line
                while(std::getline(schema_file, command)) {
                    if(!command.empty() && !command.starts_with("--")) {
                        exec_query(db, command);
                    }
                }
                exec_query(db, std::format("pragma user_version = {}", latest_schema_version));
                db.commit();
            } catch(const std::exception&) {
                db.rollback();
                throw;
            }
        }
    }
}

} // namespace sql_helpers
