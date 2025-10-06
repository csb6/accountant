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

#include <QMainWindow>

class AccountTree;
class DatabaseManager;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(AccountTree&, DatabaseManager&);
    ~MainWindow() noexcept;
signals:
    void database_path_changed(QString database_path);
public slots:
    void reset();
private slots:
    void open_transactions_view(QModelIndex account);
private:
    struct Impl;
    Impl* m_impl;
};
