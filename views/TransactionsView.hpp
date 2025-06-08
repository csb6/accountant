#pragma once

#include <QFrame>

namespace Ui {
    class TransactionsView;
}

class TransactionsView : public QFrame {
    Q_OBJECT
public:
    explicit
    TransactionsView();
    ~TransactionsView() noexcept;
private:
    Ui::TransactionsView* ui;
};
