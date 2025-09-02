#include "AccountModel.hpp"
#include <QDate>
#include "SQLColumns.hpp"

AccountModel::AccountModel(QSqlDatabase& db, int account_id)
    : QSqlRelationalTableModel(nullptr, db)
{
    setTable("transactions");
    setRelation(TRANSACTIONS_SOURCE, QSqlRelation{"accounts", "id", "name"});
    setHeaderData(TRANSACTIONS_SOURCE, Qt::Horizontal, "source");
    setRelation(TRANSACTIONS_DESTINATION, QSqlRelation{"accounts", "id", "name"});
    setHeaderData(TRANSACTIONS_DESTINATION, Qt::Horizontal, "destination");
    setFilter(QString("source = %1 or destination = %1").arg(account_id));

    select();
}

QVariant AccountModel::data(const QModelIndex& index, int role) const
{
    if(role != Qt::DisplayRole) {
        return QSqlRelationalTableModel::data(index, role);
    }

    if(index.column() == TRANSACTIONS_DATE) {
        auto isoDateStr = QSqlRelationalTableModel::data(index, role).toString();
        return QDate::fromString(isoDateStr, Qt::ISODate);
    }
    return QSqlQueryModel::data(index, role);
}
