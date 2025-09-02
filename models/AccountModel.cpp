#include "AccountModel.hpp"
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
