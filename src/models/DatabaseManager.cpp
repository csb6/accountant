#include "DatabaseManager.hpp"
#include <exception>
#include <optional>
#include <QSqlDatabase>
#include <QSqlError>
#include "util/sql_helpers.hpp"
#ifdef SQL_QUERY_LOGGING
#include <iostream>
#include <QSqlDriver>
#include <sqlite3.h>
#endif

using namespace Qt::StringLiterals;

struct DatabaseManager::Impl {
    QSqlDatabase db;
    unsigned int db_gen = 0;
};

static constexpr int latest_schema_version = 1;

DatabaseManager::DatabaseManager()
    : m_impl(new Impl)
{}

DatabaseManager::~DatabaseManager() noexcept
{
    delete m_impl;
}

QSqlDatabase& DatabaseManager::database()
{
    return m_impl->db;
}

void DatabaseManager::load_database(QString database_path)
{
    auto db_name = QString::number(m_impl->db_gen++);
    auto standby_db = QSqlDatabase::addDatabase(u"QSQLITE"_s, db_name);
    standby_db.setDatabaseName(database_path);
    std::optional<QString> error_message;
    if(!standby_db.open()) {
        error_message = standby_db.lastError().databaseText();
        if(error_message->isEmpty()) {
            error_message = u"Failed to open accounts database"_s;
        }
    } else {
        try {
            sql_helpers::upgrade_schema_if_needed(standby_db, latest_schema_version, u"schemas"_s);
            sql_helpers::exec_query(standby_db, u"pragma foreign_keys = ON"_s);
        } catch(const std::exception& err) {
            // For some reason, Qt does not check if the SQLite database that it opened is actually
            // a valid database file, so we do not find out until attempting to execute the first
            // query (which will then throw an exception)
            error_message = u"Failed to read '%1'\n(Reason: %2)"_s.arg(database_path, err.what());
        }
    }

    if(error_message.has_value()) {
        standby_db = {};
        QSqlDatabase::removeDatabase(db_name);
        emit failed_to_load_database(*error_message);
    } else {
        if(m_impl->db.isOpen()) {
            auto old_db_name = m_impl->db.connectionName();
            // Notify models so that they can close their queries
            emit database_closing();
            m_impl->db = {};
            QSqlDatabase::removeDatabase(old_db_name);
        }
        m_impl->db = standby_db;
        #ifdef SQL_QUERY_LOGGING
            auto v = m_impl->db.driver()->handle();
            if(v.isValid()) {
                auto* sqlite_handle = *static_cast<sqlite3**>(v.data());
                if(sqlite_handle) {
                    sqlite3_trace_v2(sqlite_handle, SQLITE_TRACE_STMT, [](unsigned, void*, void* p, void*) {
                        auto* stmt = static_cast<sqlite3_stmt*>(p);
                        auto* stmt_text = sqlite3_expanded_sql(stmt);
                        std::cerr << "SQLITE: " << stmt_text << "\n";
                        sqlite3_free(stmt_text);
                        return 0;
                    }, nullptr);
                }
            }
        #endif
        emit database_loaded();
    }
}
