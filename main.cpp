#include <iostream>
#include <string>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "sql_helpers.hpp"
#include "models/AccountTreeModel.hpp"
#include "views/MainWindow.hpp"

static constexpr int latest_schema_version = 1;

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("accounts.db");
    if(!db.open()) {
        std::cerr << "Error: could not open accounts database\n";
        return 1;
    }
    sql_helpers::upgrade_schema_if_needed(db, latest_schema_version, "schemas");
    sql_helpers::exec_query(db, "pragma foreign_keys = ON");

    AccountTreeModel account_model{db};
    MainWindow main_window{account_model};
    main_window.show();
    return app.exec();
}
