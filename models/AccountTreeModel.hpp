#pragma once

#include <memory>
#include <QStandardItemModel>
#include "AccountModel.hpp"

QT_BEGIN_NAMESPACE
class QSqlDatabase;
QT_END_NAMESPACE

class AccountTreeModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit
    AccountTreeModel(QSqlDatabase&);
    ~AccountTreeModel() noexcept;

    std::unique_ptr<AccountModel> account_at(QModelIndex);
private:
    struct Impl;
    Impl* m_impl;
};
