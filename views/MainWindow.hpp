#pragma once

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class AccountTreeModel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit
    MainWindow(AccountTreeModel&);
    ~MainWindow() noexcept;
private slots:
    void open_transactions_view(QModelIndex account);
private:
    Ui::MainWindow* ui;
};
