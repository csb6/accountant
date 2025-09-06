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

#include "Account.hpp"
#include <QDate>
#include "SQLColumns.hpp"

Account::Account(QSqlDatabase& db, int account_id)
    : QSqlRelationalTableModel(nullptr, db)
{
    setTable("transactions");
    setRelation(TRANSACTIONS_SOURCE, QSqlRelation{"accounts", "id", "name"});
    setHeaderData(TRANSACTIONS_SOURCE, Qt::Horizontal, "source");
    setRelation(TRANSACTIONS_DESTINATION, QSqlRelation{"accounts", "id", "name"});
    setHeaderData(TRANSACTIONS_DESTINATION, Qt::Horizontal, "destination");
    setFilter(QString("source = %1 or destination = %1").arg(account_id));
    setEditStrategy(EditStrategy::OnFieldChange);

    select();
}

QVariant Account::data(const QModelIndex& index, int role) const
{
    if(role == Qt::DisplayRole) {
        if(index.column() == TRANSACTIONS_DATE) {
            auto isoDateStr = QSqlRelationalTableModel::data(index, role).toString();
            return QDate::fromString(isoDateStr, Qt::ISODate);
        }
    }
    return QSqlRelationalTableModel::data(index, role);
}
