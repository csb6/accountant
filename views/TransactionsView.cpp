#include "TransactionsView.hpp"
#include "ui_transactionsview.h"

TransactionsView::TransactionsView()
    : QFrame(), ui(new Ui::TransactionsView())
{
    ui->setupUi(this);
}

TransactionsView::~TransactionsView() noexcept
{
    delete ui;
}
