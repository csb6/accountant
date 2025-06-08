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
    MainWindow(QWidget* parent, QAbstractItemModel& accounts_model);
    ~MainWindow() noexcept;
private:
    Ui::MainWindow* ui;
};
