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
#include <QApplication>
#include <QCommandLineParser>
#include <QErrorMessage>
#include <QOverload>
#include <QString>
#include "models/AccountTree.hpp"
#include "models/DatabaseManager.hpp"
#include "views/MainWindow.hpp"

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    QCommandLineParser parser;
    parser.setApplicationDescription("Accounting program");
    parser.addHelpOption();
    parser.addPositionalArgument("database_path", "Path of the database");
    parser.process(app);
    auto args = parser.positionalArguments();
    // TODO: if no database path provided, create a new empty database, prompt user to save
    if(args.size() != 1) {
        std::cerr << parser.helpText().toStdString() << "\n";
        return 1;
    }

    auto database_path = args[0];
    QErrorMessage error_dialog;
    DatabaseManager db_manager;
    QObject::connect(&db_manager, &DatabaseManager::failed_to_load_database,
                     &error_dialog, qOverload<const QString&>(&QErrorMessage::showMessage));
    db_manager.load_database(database_path);
    AccountTree account_tree{db_manager.database()};
    MainWindow main_window{account_tree};
    QObject::connect(&main_window, &MainWindow::database_changed, &db_manager, &DatabaseManager::load_database);
    QObject::connect(&db_manager, &DatabaseManager::database_loaded, &account_tree, &AccountTree::load);
    QObject::connect(&db_manager, &DatabaseManager::database_closing, &account_tree, &AccountTree::reset);
    QObject::connect(&db_manager, &DatabaseManager::database_closing, &main_window, &MainWindow::reset);

    main_window.show();
    return app.exec();
}
