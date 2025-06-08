#include "MainWindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent, QAbstractItemModel& accounts_model)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->account_view->setModel(&accounts_model);
}

MainWindow::~MainWindow() noexcept
{
    delete ui;
}
