#include "MainWindow.hpp"
#include "models/AccountTreeModel.hpp"
#include "TransactionsView.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(AccountTreeModel& account_tree)
    : QMainWindow(), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->account_view->setModel(&account_tree);
    auto* tab_bar = ui->tabs->tabBar();
    // Accounts tab cannot be closed
    tab_bar->setTabButton(0, QTabBar::ButtonPosition::RightSide, nullptr);
    tab_bar->setTabButton(0, QTabBar::ButtonPosition::LeftSide, nullptr);

    connect(ui->account_view, &QTreeView::activated, this, &MainWindow::open_transactions_view);
    connect(ui->tabs, &QTabWidget::tabCloseRequested, [this](int tab_index) {
        auto* tab = ui->tabs->widget(tab_index);
        ui->tabs->removeTab(tab_index);
        // Needed since removeTab doesn't delete the object
        delete tab;
    });
}

MainWindow::~MainWindow() noexcept
{
    delete ui;
}

void MainWindow::open_transactions_view(QModelIndex account)
{
    auto tab_name = account.data().toString();
    // Don't add new tab if already open (TODO: name clashes in nested accounts)
    for(int i = 0; i < ui->tabs->count(); ++i) {
        if(ui->tabs->tabText(i) == tab_name) {
            ui->tabs->setCurrentIndex(i);
            return;
        }
    }
    auto* account_tree = static_cast<AccountTreeModel*>(ui->account_view->model());
    auto account_model = account_tree->account_at(account);
    if(!account_model) {
        return;
    }
    auto* transactions_view = new TransactionsView(std::move(account_model));
    auto tab_index = ui->tabs->addTab(transactions_view, tab_name);
    ui->tabs->setCurrentIndex(tab_index);
}
