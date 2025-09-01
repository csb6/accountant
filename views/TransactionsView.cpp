#include "TransactionsView.hpp"
#include "models/AccountModel.hpp"
#include "ui_transactionsview.h"

struct TransactionsView::Impl {
    AccountModel* account;
    Ui::TransactionsView ui;
};

TransactionsView::TransactionsView(AccountModel* account)
    : QFrame(), m_impl(new Impl(account))
{
    m_impl->ui.setupUi(this);
}

TransactionsView::~TransactionsView() noexcept
{
    delete m_impl->account;
    delete m_impl;
}
