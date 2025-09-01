#pragma once

#include <QFrame>

namespace Ui {
    class TransactionsView;
}

class AccountModel;

class TransactionsView : public QFrame {
    Q_OBJECT
public:
    explicit
    TransactionsView(AccountModel*);
    ~TransactionsView() noexcept;
private:
    struct Impl;
    Impl* m_impl;
};
