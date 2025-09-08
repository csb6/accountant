#include "DatabaseManager.hpp"
#include <exception>
#include <QErrorMessage>
#include <QSqlDatabase>
#include <QSqlError>
#include "util/sql_helpers.hpp"

struct DatabaseManager::Impl {
    QSqlDatabase db;
    QErrorMessage error_dialog;
};

static constexpr int latest_schema_version = 1;

DatabaseManager::DatabaseManager(QString database_path)
    : m_impl(new Impl(QSqlDatabase::addDatabase("QSQLITE")))
{
    load_database(database_path);
}

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
    if(!m_impl->db.open()) {
        QString error_message = m_impl->db.lastError().databaseText();
        if(error_message.isEmpty()) {
            error_message = "Failed to open accounts database";
        }
        m_impl->error_dialog.showMessage(error_message);
        return;
    }
    try {
        sql_helpers::upgrade_schema_if_needed(m_impl->db, latest_schema_version, "schemas");
        sql_helpers::exec_query(m_impl->db, "pragma foreign_keys = ON");
    } catch(const std::exception&) {
        // For some reason, Qt does not check if the SQLite database that it opened is actually
        // a valid database file, so we do not find out until attempting to execute the first
        // query (which will then throw an exception)
        m_impl->error_dialog.showMessage("Failed to read accounts database");
        return;
    }
    emit database_loaded();
}
