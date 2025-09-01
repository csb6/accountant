#pragma once

#include <QSqlRelationalTableModel>
#include <QString>

class AccountModel : public QSqlRelationalTableModel {
    Q_OBJECT
public:
    explicit
    AccountModel(QString account_name);
};
