/*
QAccountant - personal accounting software
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
#include <QString>
#include "models/AccountTree.hpp"
#include "models/DatabaseManager.hpp"
#include "views/MainWindow.hpp"

using namespace Qt::StringLiterals;

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    QCommandLineParser parser;
    parser.setApplicationDescription(u"Accounting program"_s);
    parser.addHelpOption();
    parser.addPositionalArgument(u"database_path"_s, u"Path of the database"_s);
    parser.process(app);
    auto args = parser.positionalArguments();
    if(args.size() > 1) {
        std::cerr << parser.helpText().toStdString() << "\n";
        return 1;
    }
    // Empty database path means SQLite will create an in-memory database
    // that can later be saved to disk
    QString database_path;
    if(args.size() == 1) {
        database_path = args[0];
    }

    DatabaseManager db_manager;
    AccountTree account_tree{db_manager};
    MainWindow main_window{account_tree, db_manager};
    db_manager.load_database(database_path);

    main_window.show();
    return app.exec();
}
