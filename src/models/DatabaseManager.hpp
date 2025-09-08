#pragma once

#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QSqlDatabase;
QT_END_NAMESPACE

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit
    DatabaseManager(QString database_path);
    ~DatabaseManager() noexcept;
    QSqlDatabase& database();
signals:
    void database_loaded();
public slots:
    void load_database(QString database_path);
private:
    struct Impl;
    Impl* m_impl;
};
