#pragma once

#include <QSqlRelationalTableModel>
#include <QString>

class AccountModel : public QSqlRelationalTableModel {
    Q_OBJECT
public:
    AccountModel(QSqlDatabase&, int account_id);

    QVariant data(const QModelIndex&, int role) const override;
};
