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

#include "AccountTransactions.hpp"
#include <QDate>
#include "SQLColumns.hpp"

AccountTransactions::AccountTransactions(QSqlDatabase& db, int account_id)
    : QSqlRelationalTableModel(nullptr, db)
{
    setTable("transactions");
    setRelation(TRANSACTIONS_SOURCE, QSqlRelation{"accounts", "id", "name"});
    setHeaderData(TRANSACTIONS_SOURCE, Qt::Horizontal, "source");
    setRelation(TRANSACTIONS_DESTINATION, QSqlRelation{"accounts", "id", "name"});
    setHeaderData(TRANSACTIONS_DESTINATION, Qt::Horizontal, "destination");
    setFilter(QString("source = %1 or destination = %1").arg(account_id));
    // Note: can't use OnItemChange because that causes the foreign keys to be exposed
    //  when editing (instead of the human-readable names those keys are mapped to)
    setEditStrategy(EditStrategy::OnManualSubmit);
    setSort(TRANSACTIONS_DATE, Qt::SortOrder::AscendingOrder);

    select();
}

QVariant AccountTransactions::headerData(int section, Qt::Orientation orientation, int role) const
{
    auto header_data = QSqlRelationalTableModel::headerData(section, orientation, role);
    if(orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole) {
        auto col_name = header_data.toString();
        if(!col_name.isEmpty()) {
            // Capitalize first letter of each column name
            col_name[0] = col_name[0].toUpper();
        }
        header_data = col_name;
    }
    return header_data;
}

QVariant AccountTransactions::data(const QModelIndex& index, int role) const
{
    auto data = QSqlRelationalTableModel::data(index, role);
    if(role == Qt::DisplayRole || role == Qt::EditRole) {
        switch(index.column()) {
            case TRANSACTIONS_DATE:
                data = QDate::fromString(data.toString(), Qt::ISODate);
                break;
            case TRANSACTIONS_AMOUNT:
                data = data.toUInt();
                break;
            default:
                break;
        }
    }
    return data;
}
