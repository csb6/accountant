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

#pragma once

#include <memory>
#include <QStandardItemModel>
#include "AccountTransactions.hpp"

QT_BEGIN_NAMESPACE
class QSqlDatabase;
QT_END_NAMESPACE

class AccountTree : public QStandardItemModel {
    Q_OBJECT
public:
    explicit
    AccountTree(QSqlDatabase&);
    ~AccountTree() noexcept;

    std::unique_ptr<AccountTransactions> account_transactions(const QModelIndex&);
    QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex&, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;
public slots:
    void reset();
    void load();
private:
    struct Impl;
    Impl* m_impl;
};
