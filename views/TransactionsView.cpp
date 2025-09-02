#include "TransactionsView.hpp"
#include "models/AccountModel.hpp"
#include "models/SQLColumns.hpp"
#include "ui_transactionsview.h"

struct TransactionsView::Impl {
    std::unique_ptr<AccountModel> account;
    Ui::TransactionsView ui;
};

TransactionsView::TransactionsView(std::unique_ptr<AccountModel> account)
    : QFrame(), m_impl(new Impl(std::move(account)))
{
    m_impl->ui.setupUi(this);
    m_impl->ui.transactions_view->setModel(m_impl->account.get());
    m_impl->ui.transactions_view->hideColumn(TRANSACTIONS_ID);
    m_impl->ui.transactions_view->verticalHeader()->setVisible(false);
}

TransactionsView::~TransactionsView() noexcept
{
    delete m_impl;
}
