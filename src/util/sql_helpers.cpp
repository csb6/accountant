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
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <QStringTokenizer>

using namespace Qt::StringLiterals;

namespace sql_helpers {

void try_(QSqlQuery& query, bool status)
{
    if(!status) {
        throw Error(query.lastError().text().toStdString());
    }
}

QSqlQuery exec_query(QSqlDatabase& db, QString query_text)
{
    QSqlQuery query{db};
    try_(query, query.exec(query_text));
    return query;
}

void upgrade_schema_if_needed(QSqlDatabase& db, int latest_schema_version, QString schema_dir_path)
{
    QDir schema_folder{schema_dir_path};
    if(!schema_folder.exists()) {
        throw Error("Schema folder path does not exist");
    }

    auto query = exec_query(db, u"pragma user_version"_s);
    try_(query, query.next());
    auto schema_version = query.value(0).toInt();
    if(schema_version > latest_schema_version) {
        throw Error("Database has newer schema version than this software supports");
    } else if(schema_version < latest_schema_version) {
        // Migrate to the latest schema
        for(auto v = schema_version + 1; v <= latest_schema_version; ++v) {
            try {
                db.transaction();
                auto schema_filename = u"%1-schema.sql"_s.arg(v);
                auto schema_path = schema_folder.filePath(schema_filename);
                QFile schema_file{schema_path};
                if(!schema_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    throw Error(u"Schema migration failed - missing file: '%1'"_s.arg(schema_filename).toStdString());
                }
                auto schema_text = QString::fromUtf8(schema_file.readAll());
                // Each statement must be separated by two newlines
                for(auto statement : QStringTokenizer(schema_text, u"\n\n")) {
                    if(!statement.startsWith(u"--")) {
                        exec_query(db, statement.toString());
                    }
                }
                exec_query(db, u"pragma user_version = %1"_s.arg(latest_schema_version));
                db.commit();
            } catch(const std::exception&) {
                db.rollback();
                throw;
            }
        }
    }
}

} // namespace sql_helpers
