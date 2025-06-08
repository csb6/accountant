#pragma once

#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
class QSqlDatabase;
QT_END_NAMESPACE

class AccountTreeModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit
    AccountTreeModel(QSqlDatabase&);
    ~AccountTreeModel() noexcept;
private:
    struct Impl;
    Impl* m_impl;
};
