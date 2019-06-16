#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString connectionName("example");
    QSqlDatabase::addDatabase("QSQLITE", connectionName);

    // open
    {
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        db.setDatabaseName(":memory:"); // or file path
        if (!db.open()) {
            qWarning() << db.lastError();
            return __LINE__;
        }
    }

    // create table
    {
        QSqlQuery query(QSqlDatabase::database(connectionName));
        if (query.prepare("CREATE TABLE account("
                          "  id INTEGER PRIMARY KEY"
                          ", name TEXT NOT NULL"
                          ", password TEXT NOT NULL"
                          ")")) {
            if (!query.exec()) {
                qWarning() << query.lastError();
                qInfo() << query.lastQuery() << query.boundValues();
            }
        } else {
            qWarning() << query.lastError();
        }
    }

    // add
    {
        QSqlQuery query(QSqlDatabase::database(connectionName));
        if (query.prepare("INSERT INTO account(name, password)"
                          " VALUES (?, ?)")) {
            query.addBindValue("user0");
            query.addBindValue("password0"); // TODO: do not store plain password
            if (query.exec()) {
                qInfo() << query.lastInsertId().toLongLong() << "added";
            } else {
                qWarning() << query.lastError();
                qInfo() << query.lastQuery() << query.boundValues();
            }
        } else {
            qWarning() << query.lastError();
        }
    }

    // transaction
    {
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if (db.transaction()) {
            QSqlQuery query(db);
            if (query.prepare("INSERT INTO account(name, password)"
                              " VALUES (?, ?)")) {
                for (int i = 1; i < 10; i++) {
                    query.addBindValue(QString("user%1").arg(i));
                    query.addBindValue(QString("password%1").arg(i));
                    if (!query.exec()) {
                        qWarning() << query.lastError();
                        qInfo() << query.lastQuery() << query.boundValues();
                    }
                }
                if (!db.commit()) {
                    qWarning() << "commit failed" << db.lastError();
                }
            } else {
                qWarning() << query.lastError();
            }
        } else {
            qWarning() << "transaction failed" << db.lastError();
        }
    }

    // delete
    {
        QSqlQuery query(QSqlDatabase::database(connectionName));
        if (query.prepare("DELETE FROM account"
                          " WHERE name = ?")) {
            query.addBindValue("user4");
            if (!query.exec()) {
                qWarning() << query.lastError();
                qInfo() << query.lastQuery() << query.boundValues();
            }
        } else {
            qWarning() << query.lastError();
        }
    }

    // edit
    {
        QSqlQuery query(QSqlDatabase::database(connectionName));
        if (query.prepare("UPDATE account"
                          " SET password = ?"
                          " WHERE name = ?")) {
            query.addBindValue("passwordX");
            query.addBindValue("user7");
            if (!query.exec()) {
                qWarning() << query.lastError();
                qInfo() << query.lastQuery() << query.boundValues();
            }
        } else {
            qWarning() << query.lastError();
        }
    }

    // select
    {
        QSqlQuery query(QSqlDatabase::database(connectionName));
        if (query.prepare("SELECT id, name, password"
                          " FROM account")) {
            if (query.exec()) {
                while (query.next()) {
                    qlonglong id = query.value(0).toLongLong();
                    QString name = query.value(1).toString();
                    QString password = query.value(2).toString();
                    qDebug() << id << name << password;
                }
            } else {
                qWarning() << query.lastError();
                qInfo() << query.lastQuery() << query.boundValues();
            }
        } else {
            qWarning() << query.lastError();
        }
    }

    // table list
    {
        qDebug() << QSqlDatabase::database(connectionName).tables();
    }

    // close
    {
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        db.close();
    }

    QSqlDatabase::removeDatabase(connectionName);

    return 0;
}
