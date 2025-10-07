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

#include "AccountTransactions.hpp"
#include <stdexcept>
#include <vector>
#include <QDate>
#include <QString>
#include "SQLColumns.hpp"

using namespace Qt::StringLiterals;

AccountTransactions::AccountTransactions(QSqlDatabase& db, int account_id, AccountKind account_kind)
    : QSqlTableModel(nullptr, db)
{
    std::vector<QString> column_names{
        u"Date"_s,
        u"Description"_s,
        u"Source"_s,
        u"Destination"_s
    };
    switch(account_kind) {
        case ACCOUNT_KIND_BANK:
        case ACCOUNT_KIND_INCOME:
        case ACCOUNT_KIND_EXPENSE:
            setTable(u"transactions_as_cash_view"_s);
            column_names.push_back(u"Amount"_s);
            break;
        case ACCOUNT_KIND_STOCK:
            setTable(u"security_transactions_view"_s);
            column_names.push_back(u"Unit Price"_s);
            column_names.push_back(u"Quantity"_s);
            break;
        default:
            throw std::runtime_error("Unexpected account kind used with AccountTransactions model");
    }
    int column_num = TRANSACTIONS_VIEW_DATE;
    for(const auto& name : column_names) {
        setHeaderData(column_num++, Qt::Horizontal, name);
    }

    setFilter(u"source = %1 or destination = %1"_s.arg(account_id));
    // Note: can't use OnItemChange because that causes the foreign keys to be exposed
    //  when editing (instead of the human-readable names those keys are mapped to)
    setEditStrategy(EditStrategy::OnManualSubmit);
    setSort(TRANSACTIONS_VIEW_DATE, Qt::SortOrder::AscendingOrder);

    select();
}

QVariant AccountTransactions::data(const QModelIndex& index, int role) const
{
    auto data = QSqlTableModel::data(index, role);
    if(role == Qt::DisplayRole || role == Qt::EditRole) {
        switch(index.column()) {
            case TRANSACTIONS_VIEW_DATE:
                data = QDate::fromString(data.toString(), Qt::ISODate);
                break;
            default:
                break;
        }
    }
    return data;
}
