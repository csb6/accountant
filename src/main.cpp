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

#include <iostream>
#include <string>
#include <QApplication>
#include <QCommandLineParser>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QString>
#include "util/sql_helpers.hpp"
#include "models/AccountTree.hpp"
#include "views/MainWindow.hpp"

static constexpr int latest_schema_version = 1;

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    QCommandLineParser parser;
    parser.setApplicationDescription("Accounting program");
    parser.addHelpOption();
    parser.addPositionalArgument("database_path", "Path of the database");
    parser.process(app);
    auto args = parser.positionalArguments();
    if(args.size() != 1) {
        std::cerr << parser.helpText().toStdString() << "\n";
        return 1;
    }

    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(args[0]);
    if(!db.open()) {
        std::cerr << "Error: could not open accounts database\n";
        return 1;
    }
    sql_helpers::upgrade_schema_if_needed(db, latest_schema_version, "schemas");
    sql_helpers::exec_query(db, "pragma foreign_keys = ON");

    AccountTree account_tree{db};
    MainWindow main_window{account_tree};
    main_window.show();
    return app.exec();
}
