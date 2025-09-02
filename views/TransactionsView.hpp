#pragma once

#include <memory>
#include <QFrame>

class AccountModel;

class TransactionsView : public QFrame {
    Q_OBJECT
public:
    explicit
    TransactionsView(std::unique_ptr<AccountModel>);
    ~TransactionsView() noexcept;
private:
    struct Impl;
    Impl* m_impl;
};
