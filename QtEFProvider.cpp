#include "QtEFProvider.h"



namespace QtEFProvider {

///
/// \brief Append a pointer of data model to database.
/// \attention The data model will add to insert history and will remove at \a saveChange() time.
/// \param item
///
template<class T>
void EFProvider<T>::append(T *item)
{
    ItemsToInsert.append(item);
}

///
/// \brief Get the string of template type T
/// \return The name of type.
///
template<class T>
QString EFProvider<T>::getTypeName()
{
    return typeid(T).name();
}

///
/// \brief Get the table name.
/// \return The table name.
///
template<class T>
QString EFProvider<T>::getTableName()
{
    return TableName;
}

///
/// \brief Remove a data model from the database.
/// \attention The data model will add to delete history and will remove at \a saveChange() time.
/// \param item : The pointer of data model.
///
template<class T>
void EFProvider<T>::remove(T *item)
{
    ItemsToDelete.append(item);
}

///
/// \brief Get count of all data in the database.
/// \return Count of data in the database.
///
template<class T>
int EFProvider<T>::count()
{
    QString command;
    int     count = 0;

    // check database type is in use and create query.
    switch (DBMSType)
    {
    case DBType::QPSQL:
        command = "select * from " + TableName + ";";
        break;

    case DBType::QMYSQL:
    case DBType::QODBC:
    case DBType::QSQLITE:
    case DBType::QSQLITE2:
        command = "select count(id) from " + TableName + ";";
        break;

    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QOCI:
    case DBType::QTDS:
        break;
    }
    // Complete query.
    while (!openDB(true)) { QThread::msleep(500); }
    QSqlQuery q = DbSqlPtr->exec(command);

    while (q.next())
    {
        /// when use dt_QPSQL, this loop continue to end of selected data and the \a count value will count all data.
        /// when use other databases, this loop will run one time and the \a count value get the query.
        count += q.value(0).toInt();
    }
    openDB(false);
    return count;
}

///
/// \brief Get all database data as a list of the data model.
/// \return A list of the data model.
///
template<class T>
QList<T *> EFProvider<T>::toList()
{
    // save all changes befor make list
    QString    command;
    QList<T *> tempList;

    // check database type is in use and create query.
    switch (DBMSType)
    {
    case DBType::QMYSQL:
    case DBType::QODBC:
    case DBType::QPSQL:
    case DBType::QSQLITE2:
    case DBType::QSQLITE:
        command = "select * from " + TableName + ";";
        break;

    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QOCI:
    case DBType::QTDS:
        break;
    }
    // Complete query.
    while (!openDB(true)) { QThread::msleep(500); }
    QSqlQuery q = DbSqlPtr->exec(command);

    while (q.next())
    {
        QObject *temp = new T();
        for (int i = 1; i < temp->metaObject()->propertyCount(); i++)
        {
            temp->setProperty(temp->metaObject()->property(i).name(), q.value(temp->metaObject()->property(i).name()));
        }
        tempList.append(qobject_cast<T *>(temp));
    }
    openDB(false);
    foreach(T * item, tempList)
    {
        _setupRelationships(item);
    }
    return tempList;
}

template<class T>
EFProvider<T>::EFProvider(QSqlDatabase *sqlDatabase, DBType type)
{
    initialize(sqlDatabase, type);
}

///
/// \brief initialize
/// \param sqlDatabase
/// \param type
///
template<class T>
void EFProvider<T>::initialize(QSqlDatabase *sqlDatabase, DBType type, QMutex *mutex)
{
    DBMSType = type;
    DbSqlPtr     = sqlDatabase;
    Mutex   = mutex;
    QObject *objTemp = qobject_cast<QObject *>(new T);

    TableName = QString(objTemp->metaObject()->className()) + "s";
    while (!openDB(true)) { QThread::msleep(500); }
    checkTableInDb();
    openDB(false);
}

template<class T>
void EFProvider<T>::initialize(QSqlDatabase *sqlDatabase, DBType type)
{
    initialize(sqlDatabase, type, new QMutex);
}

///
/// \brief operator []
/// \param index
/// \return
///
template<class T>
T *EFProvider<T>::operator [](int index)
{
    T *item = new T();
    //
    QString  command;
    QList<T> tempList;

    // check database type is in use and create query.
    switch (DBMSType)
    {
    case DBType::QMYSQL:
    case DBType::QODBC:
    case DBType::QPSQL:
    case DBType::QSQLITE:
        command = "select * from " + TableName + ";";
        break;

    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QOCI:
    case DBType::QSQLITE2:
    case DBType::QTDS:
        break;
    }
    // Complete query.
    while (!openDB(true)) { QThread::msleep(500); }
    QSqlQuery q         = DbSqlPtr->exec(command);
    int       tempIndex = 0;

    while (q.next())
    {
        if (tempIndex++ == index)
        {
            QObject *temp = qobject_cast<QObject *>(new T);
            for (int i = 1; i < temp->metaObject()->propertyCount(); i++)
            {
                temp->setProperty(temp->metaObject()->property(i).name(), q.value(temp->metaObject()->property(i).name()));
            }
            item = qobject_cast<T *>(temp);
            return item;
        }
    }
    openDB(false);
    return item;
}

///
/// \brief Get the last data that has maximum id in database.
/// \return This method will return a pointer and the user must remove that after use.
///
template<class T>
T *EFProvider<T>::last()
{
    int topIndex = getMaxId();
    T   *objTemp = find(topIndex);

    setupRelationships(objTemp);
    return objTemp;
}

///
/// \brief Get the set of data that has top index. (topIndex - \a topLevel)
/// \param topLevel : muber of top data.
/// \return This method will return a pointer and the user must remove that after use.
///
template<class T>
QList<T *> EFProvider<T>::top(int topLevel)
{
    QList<T *> tempList;
    int        topIndex = getMaxId();
    int        lowIndex = topIndex - topLevel;

    for (int i = topIndex; i >= lowIndex; i--)
    {
        auto temp = find(i);

        setupRelationships(temp);
        tempList.append(temp);
    }
    return tempList;
}

///
/// \brief Save all changes and clear history of returns data model.
///
template<class T>
void EFProvider<T>::saveChanges()
{
    while (!openDB(true)) { QThread::msleep(500); }
    // check for delete
    foreach(T * item, ItemsToUpdate)
    {
        updateExecute(item);
        updateRelationships(item);
    }
    // check for insert
    foreach(T * item, ItemsToInsert)
    {
        createExecute(item);
    }
    foreach(T * item, ItemsToDelete)
    {
        deleteExecute(item);
    }
    openDB(false);
    ItemsToUpdate.clear();
    ItemsToInsert.clear();
    ItemsToDelete.clear();
}

///
/// \brief Find the data by specific id.
/// \param id : The specific id.
/// \return The data model with specific id. (if the query has no result, return a new pointer of the data model.)
///
template<class T>
T *EFProvider<T>::find(int id)
{
    T *temp = singleOrDefault("id='" + QString::number(id) + "'");

    return temp;
}

///
/// \brief This method will find a data with \a condition.\n If the \a condition will not have result, this method will return a new empty pointer of T.
/// \param condition : The sql query string as condition.
/// \return
///
template<class T>
T *EFProvider<T>::singleOrDefault(QString condition)
{
    QList<T *> temp = where(condition);

    if (temp.count() > 0)
        return temp.first();
    else
        return new T();
}

///
/// \brief Update a data. (The \a model should have a valid id parameter.)
/// \param model : The data that has a valid id parameter.
///
template<class T>
void EFProvider<T>::update(T *model)
{
    ItemsToUpdate.append(model);
}

///
/// \brief Find a set of data with specific \a condition.
/// \param condition: The sql query string as condition.
/// \return
///
template<class T>
QList<T *> EFProvider<T>::where(QString condition)
{
    QString    command;
    QList<T *> tempList;

    // check database type is in use and create query.
    switch (DBMSType)
    {
    case DBType::QMYSQL:
    case DBType::QODBC:
    case DBType::QPSQL:
    case DBType::QSQLITE:
        command = "select * from " + TableName + " where " + condition + ";";
        break;

    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QOCI:
    case DBType::QSQLITE2:
    case DBType::QTDS:
        break;
    }
    LocalMutex.lock();
    {
        QMutexLocker ml(Mutex);

        // Complete query.
        while (!openDB(true)) { QThread::msleep(500); }
        QSqlQuery q = DbSqlPtr->exec(command);

        while (q.next())
        {
            QObject *temp = qobject_cast<QObject *>(new T);
            for (int i = 1; i < temp->metaObject()->propertyCount(); i++)
            {
                temp->setProperty(temp->metaObject()->property(i).name(), q.value(temp->metaObject()->property(i).name()));
            }
            tempList.append(qobject_cast<T *>(temp));
        }
    }

    openDB(false);
    foreach(T * item, tempList)
    {
        setupRelationships(item);
    }
    LocalMutex.unlock();
    return tempList;
}

///
/// \brief Select a column of database.
/// \param property : The comolumn name.
/// \param where :  The sql query string as condition. (default value is empty, that means no condition)
/// \return A set of selected columnt value as string.
///
template<class T>
QStringList EFProvider<T>::select(QString property, QString where)
{
    QString     command;
    QStringList tempList;
    QString     whereCommand = "";

    if (where != "")
    {
        whereCommand = " where " + where;
    }
    // check database type is in use and create query.
    switch (DBMSType)
    {
    case DBType::QMYSQL:
    case DBType::QODBC:
    case DBType::QPSQL:
    case DBType::QSQLITE:
        command = "select " + property + " from " + TableName + whereCommand + ";";
        break;

    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QOCI:
    case DBType::QSQLITE2:
    case DBType::QTDS:
        break;
    }
    // Complete query.
    while (!openDB(true)) { QThread::msleep(500); }
    QSqlQuery q = DbSqlPtr->exec(command);

    while (q.next())
    {
        tempList.append(q.value(0).toString());
    }
    openDB(false);
    return tempList;
}

///
/// \brief this method return data as a page. (paging method)
/// \param pageIndex : index of current page (0-n)
/// \param itemPerPage : number of item per page
/// \param orderByPropertie : to sorting items consider this property.
/// \return List of item type T the count is itemPerPage and from (itemPerPage*pageIndex)
///
template<class T>
QList<T *> EFProvider<T>::getPagedData(int pageIndex, int numberOfItemPerPage, QString orderByProperty)
{
    QString    command;
    QList<T *> tempList;
    QString    pageItemLimit = QString::number(numberOfItemPerPage);
    QString    offsetValue   = QString::number(pageIndex * numberOfItemPerPage);

    // check database type is in use and create query.
    switch (DBMSType)
    {
    case DBType::QMYSQL:
    case DBType::QODBC:
    case DBType::QPSQL:
    case DBType::QSQLITE:
    case DBType::QSQLITE2:
        command = "select * from " + TableName + " order by " + orderByProperty + " limit " + pageItemLimit + " offset " + offsetValue + ";";
        break;

    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QOCI:
    case DBType::QTDS:
        break;
    }
    // Complete query.
    while (!openDB(true)) { QThread::msleep(500); }
    QSqlQuery q = DbSqlPtr->exec(command);

    while (q.next())
    {
        QObject *temp = qobject_cast<QObject *>(new T);
        for (int i = 1; i < temp->metaObject()->propertyCount(); i++)
        {
            temp->setProperty(temp->metaObject()->property(i).name(), q.value(temp->metaObject()->property(i).name()));
        }
        tempList.append(qobject_cast<T *>(temp));
    }
    openDB(false);

    foreach(T * item, tempList)
    {
        setupRelationships(item);
    }
    return tempList;
}

template<class T>
void EFProvider<T>::updateObject(QObject *modelObject)
{
    this->update(static_cast<T *>(modelObject));
    this->saveChanges();
}

template<class T>
void EFProvider<T>::createObject(QObject *modelObject)
{
    this->append(qobject_cast<T *>(modelObject));
    this->saveChanges();
}

template<class T>
QObject *EFProvider<T>::findObject(QString foreignKey)
{
    return qobject_cast<QObject *>(find(foreignKey.toInt()));
}

template<class T>
QObject *EFProvider<T>::lastObject()
{
    return qobject_cast<QObject *>(last());
}

template<class T>
QList<DatabaseModel *> EFProvider<T>::allObject()
{
    return toList();
}

template<class T>
DatabaseModel *EFProvider<T>::getById(int id)
{
    return this->find(id);
}

template<class T>
void EFProvider<T>::updateRelationships(T *item)
{
    // set relationship
    foreach(EFRelationship rel, _relatioshiplist)
    {
        DatabaseModel *itemObj   = qobject_cast<DatabaseModel *>(item);
        DatabaseModel *relObject = itemObj->getRelationship<DatabaseModel>(rel.ForeignKey);

        // check rel validation
        if (relObject->property("id").toInt() > -1)
        {
            rel.EFObject->updateObject(relObject);
        }
    }
}

template<class T>
void EFProvider<T>::addRelationships(T *item)
{
    // set relationship
    foreach(EFRelationship rel, _relatioshiplist)
    {
        DatabaseModel *itemObj   = qobject_cast<DatabaseModel *>(item);
        DatabaseModel *relObject = itemObj->getRelationship<DatabaseModel>(rel.ForeignKey);

        rel.EFObject->createObject(relObject);
        itemObj->setProperty(rel.ForeignKey.toLatin1(), relObject->property("id").toInt());
    }
}

template<class T>
void EFProvider<T>::setupRelationships(T *item)
{
    // set relationship
    foreach(EFRelationship rel, _relatioshiplist)
    {
        DatabaseModel    *itemObj   = qobject_cast<DatabaseModel *>(item);
        QString          foreignKey = itemObj->property(rel.ForeignKey.toLatin1()).toString();
        auto             temp       = rel.EFObject->findObject(foreignKey);
        RelationshipItem relItem;

        relItem.RelObject  = temp;
        relItem.ForeignKey = rel.ForeignKey;
        itemObj->addRelationship(relItem);
    }
}

///
/// \brief Get the maximum id from the database.
/// \return
///
template<class T>
int EFProvider<T>::getMaxId()
{
    QString command;

    // check database type is in use and create query.
    switch (DBMSType)
    {
    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QOCI:
    case DBType::QTDS:
        break;

    case DBType::QSQLITE2:
    case DBType::QMYSQL:
    case DBType::QODBC:
    case DBType::QPSQL:
    case DBType::QSQLITE:
        command = "select MAX(id) from %1;";
        break;
    }
    // Complete query.
    while (!openDB(true)) { QThread::msleep(500); }
    command = QString(command).arg(TableName);
    QSqlQuery q        = DbSqlPtr->exec(command);
    int       topIndex = 0;

    while (q.next())
    {
        topIndex = q.value(0).toInt();
    }
    openDB(false);
    return topIndex;
}

///
/// \brief Update a data model in the database.
/// \param obj : The scpecific data model
///
template<class T>
void EFProvider<T>::updateExecute(DatabaseModel *model)
{
    int     id      = model->property("id").toUInt();
    QString command = "";

    // check database type is in use and create query.
    switch (DBMSType)
    {
    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QOCI:
    case DBType::QSQLITE2:
    case DBType::QTDS:
        break;

    case DBType::QMYSQL:
    case DBType::QODBC:
    case DBType::QPSQL:
    case DBType::QSQLITE:
        command = "update %1 set %2 where id=" + QString::number(id) + ";";
        break;
    }
    QStringList propertiVal;

    // Complete query.
    for (int i = 1; i < model->metaObject()->propertyCount(); i++)
    {
        if (QString(model->metaObject()->property(i).name()).toLower() != "id")
        {
            QString propName  = model->metaObject()->property(i).name();
            QString propValue = model->property(model->metaObject()->property(i).name()).toString();
            propertiVal << QString("%1='%2'").arg(propName, propValue);
        }
    }
    command = QString(command).arg(TableName, propertiVal.join(","));
    //        QThread::msleep(1);
    DbSqlPtr->exec(command);
}

///
/// \brief Create a data model in the database
/// \param obj : The scpecific data model
///
template<class T>
void EFProvider<T>::createExecute(DatabaseModel *model)
{
    QString command      = "";
    QString getIdCommand = "";

    // check database type is in use and create query.
    switch (DBMSType)
    {
    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QOCI:
    case DBType::QSQLITE2:
    case DBType::QTDS:
        break;

    case DBType::QMYSQL:
        command = "insert into %1 (%2) values (%3);";
        break;

    case DBType::QODBC:
    case DBType::QPSQL:
    case DBType::QSQLITE:
        command      = "insert into %1 (%2) values (%3);";
        getIdCommand = "select MAX(id) from %1;";
        break;
    }
    QStringList propertiVal;
    QStringList propertiColm;

    // Complete query.
    for (int i = 1; i < model->metaObject()->propertyCount(); i++)
    {
        // check if the object is not a relationship object.
        if (!QString(model->metaObject()->property(i).typeName()).contains("*"))
        {
            // check if propertie is not id. Becouse the id always is a auto incremental..
            if (QString(model->metaObject()->property(i).name()).toLower() != "id")
            {
                propertiColm << model->metaObject()->property(i).name();
                propertiVal << QString("'%1'").arg(model->property(model->metaObject()->property(i).name()).toString());
            }
        }
    }
    command = QString(command).arg(TableName, propertiColm.join(","), propertiVal.join(","));
    DbSqlPtr->exec(command);
    QSqlQuery q = DbSqlPtr->exec(getIdCommand.arg(TableName));

    while (q.next())
    {
        int id = q.value(0).toInt();
        model->setProperty("id", id);
    }
}

///
/// \brief Remove the data model in the database.
/// \param obj : The scpecific data model
///
template<class T>
void EFProvider<T>::deleteExecute(DatabaseModel* model)
{
    int     id      = model->property("id").toUInt();
    QString command = "";

    // check database type is in use and create query.
    switch (DBMSType)
    {
    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QOCI:
    case DBType::QMYSQL:
    case DBType::QODBC:
    case DBType::QPSQL:
    case DBType::QSQLITE2:
    case DBType::QSQLITE:
        command = "delete from %1 where id=" + QString::number(id) + ";";
        break;

    case DBType::QTDS:
        break;
    }
    // Complete query.
    command = QString(command).arg(TableName);
    DbSqlPtr->exec(command);
}

///
/// \brief Create the table in the database. If table was exist, execute this qury will not work.
///
template<class T>
void EFProvider<T>::checkTableInDb()
{
    // create table
    QString     command = "create table " + TableName + " (%1);";
    QStringList propertiColm;
    QObject     *obj = qobject_cast<QObject *>(new T);

    for (int i = 1; i < obj->metaObject()->propertyCount(); i++)
    {
        if (QString(obj->metaObject()->property(i).name()).toLower() == "id")
        {
            switch (DBMSType)
            {
            case DBType::QDB2:
            case DBType::QIBASE:
            case DBType::QMYSQL:
            case DBType::QOCI:
            case DBType::QSQLITE2:
            case DBType::QTDS:
                break;

            case DBType::QODBC:
                propertiColm << QString(obj->metaObject()->property(i).name()) + " int IDENTITY(1,1) PRIMARY KEY";
                break;

            case DBType::QPSQL:
                propertiColm << QString(obj->metaObject()->property(i).name()) + " SERIAL PRIMARY KEY";
                break;

            case DBType::QSQLITE:
                propertiColm << QString(obj->metaObject()->property(i).name()) + " INTEGER PRIMARY KEY AUTOINCREMENT";
                break;

            }
        }
        else if (!QString(obj->metaObject()->property(i).typeName()).contains("*"))
        {
            propertiColm << QString(obj->metaObject()->property(i).name()) + " " + getType(obj->metaObject()->property(i).type());
        }
        else
        {
            propertiColm << QString(obj->metaObject()->property(i).typeName()).remove("*") + "_id int";
        }
    }
    command = QString(command).arg(propertiColm.join(','));
    DbSqlPtr->exec(command);
}

///
/// \brief Open the database path.
/// \param open : opne(true)/close(false) option.
///
template<class T>
bool EFProvider<T>::openDB(bool open)
{
    if (open)
    {
        if (DbSqlPtr->password() != "")
        {
            // check open
            if (DbSqlPtr->isOpen())
                return true;
            else
            {
                if (!DbSqlPtr->open(DbSqlPtr->userName(), DbSqlPtr->password()))
                {
                    qDebug() << Q_FUNC_INFO << "db is not open";
                    qDebug() << Q_FUNC_INFO << DbSqlPtr->lastError();
                    return false;
                }
                else
                    return true;
            }
        }
        else
        {
            // check open
            if (DbSqlPtr->isOpen())
                return true;
            else
            {
                if (!DbSqlPtr->open())
                {
                    qDebug() << Q_FUNC_INFO << "db is not open";
                    qDebug() << Q_FUNC_INFO << DbSqlPtr->lastError();
                    return false;
                }
                else
                    return true;
            }
        }
    }
    else
        return true;
}

///
/// \brief Get string of value type
/// \param type : Structure value of QVariant::Type
/// \return A type of value considering the database type.
///
template<class T>
QString EFProvider<T>::getType(QVariant::Type type)
{
    switch (DBMSType)
    {
    case DBType::QPSQL:
        switch (type)
        {
        case QVariant::Bool:
            return "boolean";

        case QVariant::Int:
            return "integer";

        case QVariant::UInt:
            return "integer";

        case QVariant::LongLong:
            return "double precision";

        case QVariant::ULongLong:
            return "double precision";

        case QVariant::Double:
            return "double precision";

        case QVariant::Char:
            return "char";

        case QVariant::String:
            return "text";

        case QVariant::Date:
            return "date";

        case QVariant::Time:
            return "time";

        case QVariant::DateTime:
            return "timestamp";

        case QVariant::Url:
            return "text";

        case QVariant::Locale:
            return "text";

        case QVariant::Invalid:
        case QVariant::Map:
        case QVariant::List:
        case QVariant::StringList:
        case QVariant::ByteArray:
        case QVariant::BitArray:
        case QVariant::Rect:
        case QVariant::RectF:
        case QVariant::Size:
        case QVariant::SizeF:
        case QVariant::Line:
        case QVariant::LineF:
        case QVariant::Point:
        case QVariant::PointF:
        case QVariant::RegularExpression:
        case QVariant::Hash:
        case QVariant::EasingCurve:
        case QVariant::Uuid:
        case QVariant::ModelIndex:
        case QVariant::PersistentModelIndex:
        case QVariant::LastCoreType:
        case QVariant::Font:
        case QVariant::Pixmap:
        case QVariant::Brush:
        case QVariant::Color:
        case QVariant::Palette:
        case QVariant::Image:
        case QVariant::Polygon:
        case QVariant::Region:
        case QVariant::Bitmap:
        case QVariant::Cursor:
        case QVariant::KeySequence:
        case QVariant::Pen:
        case QVariant::TextLength:
        case QVariant::TextFormat:
        case QVariant::Transform:
        case QVariant::Matrix4x4:
        case QVariant::Vector2D:
        case QVariant::Vector3D:
        case QVariant::Vector4D:
        case QVariant::Quaternion:
        case QVariant::PolygonF:
        case QVariant::Icon:
        case QVariant::SizePolicy:
        case QVariant::UserType:
        case QVariant::LastType:
            break;
        }
        break;

    case DBType::QODBC:
    case DBType::QSQLITE2:
    case DBType::QSQLITE:
        switch (type)
        {
        case QVariant::Bool:
            return "int";

        case QVariant::Int:
            return "int";

        case QVariant::UInt:
            return "float";

        case QVariant::LongLong:
            return "float";

        case QVariant::ULongLong:
            return "float";

        case QVariant::Double:
            return "float";

        case QVariant::Char:
            return "char";

        case QVariant::String:
            return "nvarchar(50)";

        case QVariant::Date:
            return "date";

        case QVariant::Time:
            return "time";

        case QVariant::DateTime:
            return "datetime";

        case QVariant::Url:
            return "nvarchar(50)";

        case QVariant::Locale:
            return "nvarchar";

        case QVariant::Invalid:
        case QVariant::Map:
        case QVariant::List:
        case QVariant::StringList:
        case QVariant::ByteArray:
        case QVariant::BitArray:
        case QVariant::Rect:
        case QVariant::RectF:
        case QVariant::Size:
        case QVariant::SizeF:
        case QVariant::Line:
        case QVariant::LineF:
        case QVariant::Point:
        case QVariant::PointF:
        case QVariant::RegularExpression:
        case QVariant::Hash:
        case QVariant::EasingCurve:
        case QVariant::Uuid:
        case QVariant::ModelIndex:
        case QVariant::PersistentModelIndex:
        case QVariant::LastCoreType:
        case QVariant::Font:
        case QVariant::Pixmap:
        case QVariant::Brush:
        case QVariant::Color:
        case QVariant::Palette:
        case QVariant::Image:
        case QVariant::Polygon:
        case QVariant::Region:
        case QVariant::Bitmap:
        case QVariant::Cursor:
        case QVariant::KeySequence:
        case QVariant::Pen:
        case QVariant::TextLength:
        case QVariant::TextFormat:
        case QVariant::Transform:
        case QVariant::Matrix4x4:
        case QVariant::Vector2D:
        case QVariant::Vector3D:
        case QVariant::Vector4D:
        case QVariant::Quaternion:
        case QVariant::PolygonF:
        case QVariant::Icon:
        case QVariant::SizePolicy:
        case QVariant::UserType:
        case QVariant::LastType:
            break;
        }
        break;

    case DBType::QTDS:
    case DBType::QDB2:
    case DBType::QIBASE:
    case DBType::QMYSQL:
    case DBType::QOCI:
        break;
    }
    return "none";
}
}
