#include <iostream>
#include <QApplication>
#include <QSqlDatabase>

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("accounts.db");
    if(!db.open()) {
        std::cerr << "Error: could not open accounts database\n";
        return 1;
    }

    return app.exec();
}
