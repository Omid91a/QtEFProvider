#ifndef SUNRISEDBMS_H
#define SUNRISEDBMS_H

#include <QObject>
#include <QCryptographicHash>
#include <QFile>

#include <Database/Models/FrequencyRange.h>
#include <Database/Models/PlatformName.h>
#include <Database/Models/PowerRange.h>
#include <Database/Models/PulsePRIRange.h>
#include <Database/Models/PulseWidthRange.h>
#include <Database/Models/ScanTimeRange.h>
#include <Database/Models/fanoosstaggermodel.h>
#include <Database/Models/fanooshoppingmodel.h>
#include <Database/Models/fanoospatternantennamodel.h>
//
#include "efprovider_v2.h"
#include "Models/AntennaType.h"
#include "Models/EventLog.h"
#include "Models/PRIType.h"
#include "Models/Platform.h"
#include "Models/RadarFunction.h"
#include "Models/UserRoles.h"
#include "Models/ScanType.h"
#include "Models/RadarMode.h"
#include "Models/MUser.h"
#include "Models/WellKnownTargets.h"
#include "Models/WidthType.h"
#include "Models/WellKnownTargetDocument.h"
#include "Models/servofavoritposition.h"
#include "Models/ModulationType.h"
#include "Models/FrequencyType.h"
#include "Models/PolarizationType.h"
#include "Models/fanoostargetmodel.h"
#include "Models/userdiaryreportmodel.h"

class ElephintDBMS : public QObject
{
   Q_OBJECT
public:
   struct DBMSConfigProfile
   {
      bool AntennaType               = true;
      bool EventLog                  = true;
      bool PRIType                   = true;
      bool Platform                  = true;
      bool RadarFunction             = true;
      bool UserRole                  = true;
      bool ScanType                  = true;
      bool RadarMode                 = true;
      bool MUser                     = true;
      bool WellKnownTarget           = true;
      bool WidthType                 = true;
      bool WellKnownTargetDocument   = true;
      bool ServoFavoritPosition      = true;
      bool ModulationType            = true;
      bool FrequencyType             = true;
      bool PolarizationType          = true;
      bool Countries                 = true;
      bool PlatformName              = true;
      bool FrequencyRange            = true;
      bool PulseWidthRange           = true;
      bool PulsePRIRange             = true;
      bool ScanTimeRange             = true;
      bool PowerRange                = true;
      bool RadarProfileModel         = true;
      bool UserDiaryReportModel      = true;
      bool FanoosTargetModel         = true;
      bool FanoosStaggerModel        = true;
      bool FanoosHoppingModel        = true;
      bool FanoosPatternAntennaModel = true;
   };
   static ElephintDBMS *getInstance(DBMSConfigProfile profile, bool isRef = false);
   void backup();

   EFProvider_v2<AntennaType> efAntennaTypes;
   EFProvider_v2<EventLog> efEvents;
   EFProvider_v2<PRIType> efPRITypes;
   EFProvider_v2<Platform> efPlatforms;
   EFProvider_v2<RadarFunction> efRadarFunctions;
   EFProvider_v2<UserRole> efRoles;
   EFProvider_v2<ScanType> efScanTypes;
   EFProvider_v2<RadarMode> efRadarMode;
   EFProvider_v2<MUser> efUsers;
   EFProvider_v2<WellKnownTarget> efWellKnownTargets;
   EFProvider_v2<WidthType> efWidthTypes;
   EFProvider_v2<WellKnownTargetDocument> efWellKnownTargetDocuments;
   EFProvider_v2<ServoFavoritPosition> efServoFavoritPositions;
   EFProvider_v2<ModulationType> efModulationTypes;
   EFProvider_v2<FrequencyType> efFrequencyTypes;
   EFProvider_v2<PolarizationType> efPolarizationTypes;
   EFProvider_v2<Countries> efCountries;
   EFProvider_v2<PlatformName> efPlatformNames;
   EFProvider_v2<FrequencyRange> efFrequencyRanges;
   EFProvider_v2<PulseWidthRange> efPulseWidthRanges;
   EFProvider_v2<PulsePRIRange> efPulsePRIRanges;
   EFProvider_v2<ScanTimeRange> efScanTimeRanges;
   EFProvider_v2<PowerRange> efPowerRanges;
   EFProvider_v2<RadarProfileModel> efRadarProfileModel;
   EFProvider_v2<FanoosTargetModel> efFanoosTargets;
   EFProvider_v2<FanoosStaggerModel> efFanoosStaggers;
   EFProvider_v2<FanoosHoppingModel> efFanoosHoppings;
   EFProvider_v2<FanoosPatternAntennaModel> efFanoosAntennaPatterns;
   EFProvider_v2<UserDiaryReportModel> efUserDiaryReports;

   QList<EFProviderPresenter *> efProviders;
   QSqlDatabase _dbs;
   QMutex *m_dbMutex = nullptr;
   template<class ModelType>
   EFProvider_v2<ModelType> *getProvider()
   {
      foreach(EFProviderPresenter * item, efProviders)
      {
         if (item->type() == typeid(ModelType).name())
         {
            return (EFProvider_v2<ModelType> *)(item);
         }
      }
      return nullptr;
   }

   bool getState();

   ~ElephintDBMS()
   {
   }

private:
   explicit ElephintDBMS(bool init = true);
   void _initAntennaTypes();
   void _initPRITypes();
   void _initPlatforms();
   void _initRadarFunctions();
   void _initUserRoles();
   void _initScanTypes();
   void _initUsers();
   void _initWidthTypes();
   void _initialize();
   void _checkTableValues();
   void _initModulations();
   void _initFrequencyTypes();
   void _initPolarizations();
   void _initCountries();
   void _setupRelationships();

   static ElephintDBMS *_instance_;
   DBMSConfigProfile _configProfile;
//   QString dbUser  = "eLePhInt";
   QString dbUser  = "";
   QString dbPass  = "";
   QString dbName  = "ElephintDB.db";
   QString dbSever = "Elephint";
   EFProviderTypev2::DBType dbType = EFProviderTypev2::dt_QSQLITE;

   void setupDb(QString hostName, QString databaseName, EFProviderTypev2::DBType type, QString user, QString pass);

signals:
   void dbmsConnectionStateCahnged(bool state);
};

#endif // SUNRISEDBMS_H
