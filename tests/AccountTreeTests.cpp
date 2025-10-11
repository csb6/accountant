#include <iterator>
#include <QTest>
#include "DatabaseManager.hpp"
#include "SQLColumns.hpp"
#include "Roles.hpp"
#include "AccountTree.hpp"

using namespace Qt::StringLiterals;

class AccountTreeTests : public QObject {
    Q_OBJECT

    DatabaseManager db_manager;
private slots:
    void empty_tree()
    {
        AccountTree tree{db_manager};
  
        QCOMPARE(tree.rowCount(), 0);
        QCOMPARE(tree.columnCount(), 0);
    }

    void default_accounts()
    {
        AccountTree tree{db_manager};
        db_manager.load_database(u":memory:"_s);

        static const struct {
            QString account_name;
            int account_id;
        } expected_rows[] = {
            {u"Assets"_s,      1},
            {u"Equity"_s,      2},
            {u"Expenses"_s,    3},
            {u"Income"_s,      4},
            {u"Liabilities"_s, 5},
        };

        QCOMPARE(tree.rowCount(), std::size(expected_rows));
        QCOMPARE(tree.columnCount(), 1);

        for(int row = 0; row < tree.rowCount(); ++row) {
            auto index = tree.index(row, 0);
            QCOMPARE(index.data(), expected_rows[row].account_name);
            QCOMPARE(index.data(Account_ID_Role), expected_rows[row].account_id);
            QCOMPARE(index.data(Account_Path_Role), expected_rows[row].account_name);
            QCOMPARE(index.data(Account_Kind_Role), ACCOUNT_KIND_PLACEHOLDER);
        }
    }

    void child_account()
    {
        AccountTree tree{db_manager};
        db_manager.load_database(u":memory:"_s);

        auto assets = tree.index(0, 0);
        tree.appendRow(AccountFields{AccountFields{u"Checking"_s, u""_s, ACCOUNT_KIND_BANK}}, assets);
        auto child_index = tree.index(0, 0, assets);
        QCOMPARE(child_index.data(), u"Checking"_s);
        QCOMPARE(child_index.data(Account_ID_Role), 6);
        QCOMPARE(child_index.data(Account_Path_Role), u"Assets:Checking"_s);
        QCOMPARE(child_index.data(Account_Kind_Role), ACCOUNT_KIND_BANK);
    }
};

QTEST_MAIN(AccountTreeTests)
#include "AccountTreeTests.moc"
