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
#pragma once

#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QSqlDatabase;
QT_END_NAMESPACE

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    DatabaseManager();
    ~DatabaseManager() noexcept;
    QSqlDatabase& database();
signals:
    void database_closing();
    void database_loaded();
    void failed_to_load_database(QString error_message);
public slots:
    void load_database(QString database_path);
private:
    struct Impl;
    Impl* m_impl;
};
