#ifndef DATABASEMODEL_H
#define DATABASEMODEL_H

#include <QMap>
#include <QMetaProperty>

struct RelationshipItem
{
   RelationshipItem() {}
   QString ForeignKey = "";
   QObject *RelObject = nullptr;
};
class DatabaseModel : public QObject
{
   Q_OBJECT
public:
   QMap<QString, QVariant> getAllPropertie()
   {
      QMap<QString, QVariant> returnValue;

      for (int i = 0; i < this->metaObject()->propertyCount(); i++)
      {
         QString  propName  = this->metaObject()->property(i).name();
         QVariant propValue = this->property(this->metaObject()->property(i).name());
         returnValue.insert(propName, propValue);
      }
      return returnValue;
   }

   QVariant getPropertie(QString key)
   {
      return this->property(key.toUtf8());
   }

   QList<RelationshipItem> _relationshipList;
   void addRelationship(RelationshipItem foreignKey)
   {
      _relationshipList.append(foreignKey);
   }

   template<class T>
   T *getRelationship(QString foreignKey)
   {
      foreach(RelationshipItem rel, _relationshipList)
      {
         if (rel.ForeignKey == foreignKey)
         {
            return static_cast<T *>(rel.RelObject);
         }
      }
      // if not found any rel, create alternative relationship
      RelationshipItem newRel;

      newRel.RelObject  = new T();
      newRel.ForeignKey = foreignKey;
      _relationshipList.append(newRel);
      return static_cast<T *>(newRel.RelObject);
   }
};

#endif // DATABASEMODEL_H
