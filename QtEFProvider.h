#ifndef QTEFPROVIDER_H
#define QTEFPROVIDER_H

#include "databasemodel.h"
#include <QList>
#include <QSqlQuery>
#include <QDebug>
#include <QMetaProperty>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QThread>
#include <QMutex>

namespace QtEFProvider {
///
/// \brief The DatabaseType enum
///
enum class DBType : int
{
    QDB2,               /// IBM DB2
    QIBASE,             /// Borland InterBase Driver
    QMYSQL,             /// MySQL Driver
    QOCI,               /// Oracle Call Interface Driver
    QODBC,              /// ODBC Driver (includes Microsoft SQL Server)
    QPSQL,              /// PostgreSQL Driver
    QSQLITE,            /// SQLite version 3 or above
    QSQLITE2,           /// SQLite version 2
    QTDS                /// Sybase Adaptive Server
};

class EFProviderPresenter
{
public:
    struct EFRelationship
    {
        EFRelationship() {}
        EFRelationship(QString fkey, EFProviderPresenter *efObject)
        {
            ForeignKey = fkey;
            EFObject   = efObject;
        }

        QString             ForeignKey = "";
        EFProviderPresenter *EFObject  = nullptr;
    };
    virtual QString getTypeName()      = 0;
    virtual QString getTableName() = 0;

    void setRelationship(EFRelationship rel)
    {
        _relatioshiplist.append(rel);
    }

    virtual void updateObject(QObject *modelObject) = 0;
    virtual void createObject(QObject *modelObject) = 0;
    virtual QObject *findObject(QString foreignKey) = 0;
    virtual QObject *lastObject() = 0;
    virtual QList<DatabaseModel *> allObject() = 0;
    virtual DatabaseModel *getById(int id)     = 0;

protected:
    QList<EFRelationship> _relatioshiplist;
};


template<class T>
class EFProvider : public EFProviderPresenter
{
public:
    explicit EFProvider(){}
    explicit EFProvider(QSqlDatabase *sqlDatabase, DBType type = DBType::QODBC);

    void append(T *item);
    QString getTypeName();
    QString getTableName();
    void remove(T *item);
    int count();
    QList<T *> toList();
    void initialize(QSqlDatabase *sqlDatabase, DBType type, QMutex *mutex);
    void initialize(QSqlDatabase *sqlDatabase, DBType type = DBType::QODBC);
    T *operator [](int index);
    T *last();
    QList<T *> top(int topLevel);
    void saveChanges();
    T *find(int id);
    T *singleOrDefault(QString condition);
    void update(T *model);
    QList<T *> where(QString condition);
    QStringList select(QString property, QString where = "");
    QList<T *> getPagedData(int pageIndex, int numberOfItemPerPage, QString orderByProperty = "id");

private:
    QSqlDatabase *DbSqlPtr;
    QList<T *> BaseTableItems;
    DBType DBMSType;
    QString TableName;
    QString DBSName;
    QString HostName;
    QMutex *Mutex = nullptr;
    QMutex LocalMutex;

    /// The history of data sets for update in the database
    QList<T *> ItemsToUpdate;
    /// The history of data sets for delete of the database
    QList<T *> ItemsToDelete;
    /// The history of data sets for insert in the database
    QList<T *> ItemsToInsert;

private:
    void updateObject(QObject *modelObject);
    void createObject(QObject *modelObject);
    QObject *findObject(QString foreignKey);
    QObject *lastObject();
    QList<DatabaseModel *> allObject();
    DatabaseModel *getById(int id);

    void updateRelationships(T *item);
    void addRelationships(T *item);
    void setupRelationships(T *item);

    int getMaxId();
    void updateExecute(DatabaseModel *model);
    void createExecute(DatabaseModel *obj);
    void deleteExecute(DatabaseModel *obj);
    void checkTableInDb();
    bool openDB(bool open);
    QString getType(QVariant::Type type);
};

}

#endif // QTEFPROVIDER_H
