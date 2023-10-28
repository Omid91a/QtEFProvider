
#include "qtefprovidertest.h"

QtEFProviderTest::QtEFProviderTest(QObject *parent)
    : QObject{parent}
{
    QSqlDatabase* sql_obj = new QSqlDatabase(buildSqlObject("","country_db",QtEFProvider::DBType::QSQLITE2));
    EfCountry.initialize(sql_obj,QtEFProvider::DBType::QSQLITE2);

}


QString getDbType(QtEFProvider::DBType type)
{
    switch (type)
    {
    case QtEFProvider::DBType::QDB2:
        return "QDB2";

    case QtEFProvider::DBType::QIBASE:
        return "QIBASE";

    case QtEFProvider::DBType::QMYSQL:
        return "QMYSQL";

    case QtEFProvider::DBType::QOCI:
        return "QOCI";

    case QtEFProvider::DBType::QODBC:
        return "QODBC";

    case QtEFProvider::DBType::QPSQL:
        return "QPSQL";

    case QtEFProvider::DBType::QSQLITE:
        return "QSQLITE";

    case QtEFProvider::DBType::QSQLITE2:
        return "QSQLITE2";

    case QtEFProvider::DBType::QTDS:
        return "QTDS";
    }
    return "none";
}

QSqlDatabase QtEFProviderTest::buildSqlObject(QString hostName, QString databaseName, QtEFProvider::DBType type, QString user, QString pass)
{
    QString connectionStr;
    QString connectionStrDbName;

    auto local_db = QSqlDatabase::addDatabase(getDbType(type));
    // check database type is in use and create connection query.
    switch (type)
    {
    case QtEFProvider::DBType::QDB2:
    case QtEFProvider::DBType::QIBASE:
    case QtEFProvider::DBType::QMYSQL:
    case QtEFProvider::DBType::QOCI:
    case QtEFProvider::DBType::QTDS:
        break;

    case QtEFProvider::DBType::QODBC:
        connectionStr       = "DRIVER={SQL Server};Server=%1;";
        connectionStr       = QString(connectionStr).arg(hostName);
        connectionStrDbName = "DRIVER={SQL Server};Server=%1;Database=%2;Uid=%3;Pwd=%4";
        connectionStrDbName = QString(connectionStrDbName).arg(hostName, databaseName, user, pass);
        break;

    case QtEFProvider::DBType::QPSQL:
        local_db.setHostName(hostName);
        local_db.setPort(5432);
        databaseName        = databaseName.toLower();
        connectionStrDbName = databaseName;
        break;

    case QtEFProvider::DBType::QSQLITE2:
    case QtEFProvider::DBType::QSQLITE:
        local_db.setHostName(hostName);
        connectionStrDbName = databaseName.toLower();
        break;
    }
    //
    local_db.setDatabaseName(connectionStr);
    if (pass != "")
    {
        local_db.setUserName(user);
        local_db.setPassword(pass);
    }
    while (local_db.isOpen())
    {
        QThread::msleep(500);
    }
    local_db.open();
    QString command;

    // create database
    command = "create database " + databaseName + ";";
    local_db.exec(command);
    local_db.close();
    local_db.setDatabaseName(connectionStrDbName);

    return local_db;
}
