#pragma once

#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QSqlDatabase;
QT_END_NAMESPACE

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    DatabaseManager();
    ~DatabaseManager() noexcept;
    QSqlDatabase& database();
signals:
    void database_closing();
    void database_loaded();
    void failed_to_load_database(QString error_message);
public slots:
    void load_database(QString database_path);
private:
    struct Impl;
    Impl* m_impl;
};
