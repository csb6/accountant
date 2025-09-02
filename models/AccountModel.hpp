#pragma once

#include <QSqlRelationalTableModel>
#include <QString>

class AccountModel : public QSqlRelationalTableModel {
    Q_OBJECT
public:
    AccountModel(QSqlDatabase&, int account_id);
};
