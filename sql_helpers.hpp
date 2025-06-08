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
