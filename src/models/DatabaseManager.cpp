#include "DatabaseManager.hpp"
#include <exception>
#include <optional>
#include <QSqlDatabase>
#include <QSqlError>
#include "util/sql_helpers.hpp"

struct DatabaseManager::Impl {
    QSqlDatabase db;
};

static constexpr int latest_schema_version = 1;

DatabaseManager::DatabaseManager()
    : m_impl(new Impl(QSqlDatabase::addDatabase("QSQLITE")))
{}

DatabaseManager::~DatabaseManager() noexcept
{}

QSqlDatabase& DatabaseManager::database()
{
    return m_impl->db;
}

void DatabaseManager::load_database(QString database_path)
{
    // TODO: if failed to load, reload previous database
    if(m_impl->db.isOpen()) {
        m_impl->db.close();
    }
    m_impl->db.setDatabaseName(database_path);
    std::optional<QString> error_message;
    if(!m_impl->db.open()) {
        error_message = m_impl->db.lastError().databaseText();
        if(error_message->isEmpty()) {
            error_message = "Failed to open accounts database";
        }
    } else {
        try {
            sql_helpers::upgrade_schema_if_needed(m_impl->db, latest_schema_version, "schemas");
            sql_helpers::exec_query(m_impl->db, "pragma foreign_keys = ON");
        } catch(const std::exception& err) {
            // For some reason, Qt does not check if the SQLite database that it opened is actually
            // a valid database file, so we do not find out until attempting to execute the first
            // query (which will then throw an exception)
            error_message = err.what();
        }
    }

    if(error_message.has_value()) {
        emit failed_to_load_database(*error_message);
    } else {
        emit database_loaded();
    }
}
