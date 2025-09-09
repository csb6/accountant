#include "DatabaseManager.hpp"
#include <exception>
#include <optional>
#include <QSqlDatabase>
#include <QSqlError>
#include "util/sql_helpers.hpp"

struct DatabaseManager::Impl {
    QSqlDatabase db;
    unsigned int db_gen = 0;
};

static constexpr int latest_schema_version = 1;

DatabaseManager::DatabaseManager()
    : m_impl(new Impl)
{}

DatabaseManager::~DatabaseManager() noexcept
{}

QSqlDatabase& DatabaseManager::database()
{
    return m_impl->db;
}

void DatabaseManager::load_database(QString database_path)
{
    auto standby_db = QSqlDatabase::addDatabase("QSQLITE", QString::number(m_impl->db_gen++));
    standby_db.setDatabaseName(database_path);
    std::optional<QString> error_message;
    if(!standby_db.open()) {
        error_message = standby_db.lastError().databaseText();
        if(error_message->isEmpty()) {
            error_message = "Failed to open accounts database";
        }
    } else {
        try {
            sql_helpers::upgrade_schema_if_needed(standby_db, latest_schema_version, "schemas");
            sql_helpers::exec_query(standby_db, "pragma foreign_keys = ON");
        } catch(const std::exception& err) {
            // For some reason, Qt does not check if the SQLite database that it opened is actually
            // a valid database file, so we do not find out until attempting to execute the first
            // query (which will then throw an exception)
            error_message = err.what();
        }
    }

    if(error_message.has_value()) {
        standby_db.close();
        QSqlDatabase::removeDatabase(standby_db.connectionName());
        emit failed_to_load_database(*error_message);
    } else {
        if(m_impl->db.isOpen()) {
            // Notify models so that they can close their queries
            emit database_closing();
            m_impl->db.close();
            QSqlDatabase::removeDatabase(m_impl->db.connectionName());
        }
        m_impl->db = QSqlDatabase::database(standby_db.connectionName());
        emit database_loaded();
    }
}
