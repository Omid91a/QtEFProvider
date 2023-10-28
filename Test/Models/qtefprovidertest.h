
#ifndef QTEFPROVIDERTEST_H
#define QTEFPROVIDERTEST_H


#include <QObject>
#include <QDebug>
#include <Test/Models/countries.h>
#include <QtEFProvider.h>


class QtEFProviderTest : public QObject
{
    Q_OBJECT
public:
    explicit QtEFProviderTest(QObject *parent = nullptr);

    QtEFProvider::EFProvider<Countries> EfCountry;

private:
    QSqlDatabase buildSqlObject(QString hostName, QString databaseName, QtEFProvider::DBType type = QtEFProvider::DBType::QSQLITE, QString user = "", QString pass = "");
signals:

};

#endif // QTEFPROVIDERTEST_H


int main()
{
    QtEFProviderTest temp;
    Countries c;
    c.setDisplay("Canada in west");
    c.setName("CA");
    qDebug()<<"Countries count: "<< temp.EfCountry.count();
    temp.EfCountry.append(&c);
    temp.EfCountry.saveChanges();
    qDebug()<<"Countries count: "<< temp.EfCountry.count();
    return 0;
}
