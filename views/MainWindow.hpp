#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
QT_END_NAMESPACE

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit
    MainWindow(QAbstractItemModel& accounts_model);
    ~MainWindow() noexcept;
private slots:
    void open_transactions_view(QModelIndex account);
private:
    Ui::MainWindow* ui;
};
