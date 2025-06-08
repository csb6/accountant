#pragma once

#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
class QSqlDatabase;
QT_END_NAMESPACE

class AccountsModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit
    AccountsModel(QSqlDatabase&);
    ~AccountsModel() noexcept;
private:
    struct Impl;
    Impl* m_impl;
};
