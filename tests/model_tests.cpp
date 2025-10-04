#include <QTest>

class TestAccountTree : public QObject {
    Q_OBJECT
};

QTEST_MAIN(TestAccountTree)
#include "model_tests.moc"
