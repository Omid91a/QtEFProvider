#include "elephintdbms.h"
#include "Database/efprovider_v2.h"

#include <QApplication>
#include <QSettings>

ElephintDBMS *ElephintDBMS::_instance_ = nullptr;

ElephintDBMS *ElephintDBMS::getInstance(DBMSConfigProfile profile, bool isRef)
{
   Q_UNUSED(profile);
   QSettings ss("ElephintDBMSAddr", nullptr);

   if (_instance_ == nullptr)
   {
      if (isRef)
      {
         _instance_ = new ElephintDBMS;
         typedef unsigned long long oaull;
         ss.setValue("addr", oaull(_instance_));
      }
      else if ((ss.value("addr").toULongLong() > 0))
      {
         uint *addr = (uint *)(ss.value("addr").toULongLong());

         _instance_ = (ElephintDBMS *)addr;
      }
      else
      {
         _instance_ = new ElephintDBMS;
      }
   }
   return _instance_;
}


void ElephintDBMS::backup()
{
   switch (dbType)
   {
   case EFProviderTypev2::dt_QDB2:
      break;

   case EFProviderTypev2::dt_QIBASE:
      break;

   case EFProviderTypev2::dt_QMYSQL:
      break;

   case EFProviderTypev2::dt_QOCI:
      break;

   case EFProviderTypev2::dt_QODBC:
      break;

   case EFProviderTypev2::dt_QPSQL:
      break;

   case EFProviderTypev2::dt_QSQLITE:
   case EFProviderTypev2::dt_QSQLITE2:
      QFile::copy(dbName, "dbBackup-" + QLocale(QLocale::English).toString(QDateTime::currentDateTime(), "yyyyMMdd HHmmss") + ".db");
      break;

   case EFProviderTypev2::dt_QTDS:
      break;
   }
}


bool ElephintDBMS::getState()
{
   // selected a db randomly.
//    return efPRITypes->getDbState();
   return true;
}


ElephintDBMS::ElephintDBMS(bool init) : QObject()
{
   if (init)
   {
      _initialize();
      _checkTableValues();
   }
}


///
/// \brief Get string of a database type.
/// \param type : The DBType structure value.
/// \return A string that is the QSqlDatabase type.
///
QString _getDbType(EFProviderTypev2::DBType type)
{
   switch (type)
   {
   case EFProviderTypev2::dt_QDB2:
      return "QDB2";

   case EFProviderTypev2::dt_QIBASE:
      return "QIBASE";

   case EFProviderTypev2::dt_QMYSQL:
      return "QMYSQL";

   case EFProviderTypev2::dt_QOCI:
      return "QOCI";

   case EFProviderTypev2::dt_QODBC:
      return "QODBC";

   case EFProviderTypev2::dt_QPSQL:
      return "QPSQL";

   case EFProviderTypev2::dt_QSQLITE:
      return "QSQLITE";

   case EFProviderTypev2::dt_QSQLITE2:
      return "QSQLITE2";

   case EFProviderTypev2::dt_QTDS:
      return "QTDS";
   }
   return "none";
}


void ElephintDBMS::setupDb(QString hostName, QString databaseName, EFProviderTypev2::DBType type = EFProviderTypev2::dt_QSQLITE, QString user = "", QString pass = "")
{
   QString connectionStr;
   QString connectionStrDbName;

   _dbs = QSqlDatabase::addDatabase(_getDbType(type));
   // check database type is in use and create connection query.
   switch (type)
   {
   case EFProviderTypev2::dt_QDB2:
   case EFProviderTypev2::dt_QIBASE:
   case EFProviderTypev2::dt_QMYSQL:
   case EFProviderTypev2::dt_QOCI:
   case EFProviderTypev2::dt_QTDS:
      break;

   case EFProviderTypev2::dt_QODBC:
      connectionStr       = "DRIVER={SQL Server};Server=%1;";
      connectionStr       = QString(connectionStr).arg(hostName);
      connectionStrDbName = "DRIVER={SQL Server};Server=%1;Database=%2;Uid=%3;Pwd=%4";
      connectionStrDbName = QString(connectionStrDbName).arg(hostName, databaseName, user, pass);
      break;

   case EFProviderTypev2::dt_QPSQL:
      _dbs.setHostName(hostName);
      _dbs.setPort(5432);
      databaseName        = databaseName.toLower();
      connectionStrDbName = databaseName;
      break;

   case EFProviderTypev2::dt_QSQLITE2:
   case EFProviderTypev2::dt_QSQLITE:
      _dbs.setHostName(hostName);
      connectionStrDbName = databaseName.toLower();
      break;
   }
   //
   _dbs.setDatabaseName(connectionStr);
   if (pass != "")
   {
      _dbs.setUserName(user);
      _dbs.setPassword(pass);
   }
   while (_dbs.isOpen())
   {
      QThread::msleep(500);
   }
   _dbs.open();
   QString command;

   // create database
   command = "create database " + databaseName + ";";
   _dbs.exec(command);
   _dbs.close();
   _dbs.setDatabaseName(connectionStrDbName);
}


///
/// \brief SunriseDBMS::_initialize
///
void ElephintDBMS::_initialize()
{
   //    dbName = "ToluDb";
   //    dbSever = "otfp.site";
   //    EFProviderType::DatabaseType dbType = EFProviderType::dt_QODBC;

//   _dbs = QSqlDatabase::addDatabase(_getDbType(_dbsType), _tableName);
   m_dbMutex = new QMutex();
   QString dbAddress = QApplication::applicationDirPath() + "/" + dbName;

   setupDb(dbSever, dbAddress, dbType, dbUser, dbPass);
   // types
   if (_configProfile.AntennaType)
   {
      efAntennaTypes.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efAntennaTypes;
   }
   if (_configProfile.PRIType)
   {
      efPRITypes.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efPRITypes;
   }
   if (_configProfile.Platform)
   {
      efPlatforms.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efPlatforms;
   }
   if (_configProfile.RadarFunction)
   {
      efRadarFunctions.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efRadarFunctions;
   }
   if (_configProfile.UserRole)
   {
      efRoles.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efRoles;
   }
   if (_configProfile.ScanType)
   {
      efScanTypes.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efScanTypes;
   }
   if (_configProfile.WidthType)
   {
      efWidthTypes.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efWidthTypes;
   }
   if (_configProfile.PlatformName)
   {
      efPlatformNames.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efPlatformNames;
   }
   //
   if (_configProfile.EventLog)
   {
      efEvents.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efEvents;
   }
   if (_configProfile.RadarMode)
   {
      efRadarMode.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efRadarMode;
   }
   if (_configProfile.MUser)
   {
      efUsers.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efUsers;
   }
   if (_configProfile.WellKnownTarget)
   {
      efWellKnownTargets.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efWellKnownTargets;
   }
   if (_configProfile.WellKnownTargetDocument)
   {
      efWellKnownTargetDocuments.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efWellKnownTargetDocuments;
   }
   if (_configProfile.ServoFavoritPosition)
   {
      efServoFavoritPositions.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efServoFavoritPositions;
   }

   if (_configProfile.ModulationType)
   {
      efModulationTypes.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efModulationTypes;
   }
   if (_configProfile.FrequencyType)
   {
      efFrequencyTypes.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efFrequencyTypes;
   }
   if (_configProfile.PolarizationType)
   {
      efPolarizationTypes.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efPolarizationTypes;
   }
   if (_configProfile.Countries)
   {
      efCountries.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efCountries;
   }
   // Ranges
   if (_configProfile.FrequencyRange)
   {
      efFrequencyRanges.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efFrequencyRanges;
   }
   if (_configProfile.PulseWidthRange)
   {
      efPulseWidthRanges.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efPulseWidthRanges;
   }
   if (_configProfile.ScanTimeRange)
   {
      efScanTimeRanges.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efScanTimeRanges;
   }
   if (_configProfile.PulsePRIRange)
   {
      efPulsePRIRanges.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efPulsePRIRanges;
   }
   if (_configProfile.PowerRange)
   {
      efPowerRanges.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efPowerRanges;
   }
   if (_configProfile.RadarProfileModel)
   {
      efRadarProfileModel.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efRadarProfileModel;
   }
   if (_configProfile.FanoosTargetModel)
   {
      efFanoosTargets.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efFanoosTargets;
   }
   if (_configProfile.FanoosStaggerModel)
   {
      efFanoosStaggers.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efFanoosStaggers;
   }
   if (_configProfile.FanoosHoppingModel)
   {
      efFanoosHoppings.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efFanoosHoppings;
   }
   if (_configProfile.FanoosPatternAntennaModel)
   {
      efFanoosAntennaPatterns.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efFanoosAntennaPatterns;
   }
   if (_configProfile.UserDiaryReportModel)
   {
      efUserDiaryReports.initialize(&_dbs, dbType, m_dbMutex);
      efProviders << &efUserDiaryReports;
   }
   _setupRelationships();
}


void ElephintDBMS::_setupRelationships()
{
   //
   efUsers.setRelationship(EFProviderPresenter::EFRelationship("roleId", &efRoles));
   //
   efEvents.setRelationship(EFProviderPresenter::EFRelationship("userId", &efUsers));
   //
   efFanoosTargets.setRelationship(EFProviderPresenter::EFRelationship("staggerId", &efFanoosStaggers));
   efFanoosTargets.setRelationship(EFProviderPresenter::EFRelationship("hoppingId", &efFanoosHoppings));
   //
   efFanoosTargets.setRelationship(EFProviderPresenter::EFRelationship("antennaPatternId", &efFanoosAntennaPatterns));
   //
   efFrequencyRanges.setRelationship(EFProviderPresenter::EFRelationship("wellknownTargetId", &efWellKnownTargets));
   efFrequencyRanges.setRelationship(EFProviderPresenter::EFRelationship("radarModeId", &efRadarMode));
   efPowerRanges.setRelationship(EFProviderPresenter::EFRelationship("wellknownTargetId", &efWellKnownTargets));
   efPowerRanges.setRelationship(EFProviderPresenter::EFRelationship("radarModeId", &efRadarMode));
   efPulsePRIRanges.setRelationship(EFProviderPresenter::EFRelationship("wellknownTargetId", &efWellKnownTargets));
   efPulsePRIRanges.setRelationship(EFProviderPresenter::EFRelationship("radarModeId", &efRadarMode));
   efPulseWidthRanges.setRelationship(EFProviderPresenter::EFRelationship("wellknownTargetId", &efWellKnownTargets));
   efPulseWidthRanges.setRelationship(EFProviderPresenter::EFRelationship("radarModeId", &efRadarMode));

   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("wellknownTargetId", &efWellKnownTargets));
   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("eventId", &efEvents));
   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("widthTypeId", &efWidthTypes));
   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("priTypeId", &efPRITypes));
   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("scanTypeId", &efScanTypes));
   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("platformId", &efPlatforms));
   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("radarFunctionId", &efRadarFunctions));
//   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("nameId", &ef));
   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("frequencyTypeId", &efFrequencyTypes));
   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("countryId", &efCountries));
   efRadarMode.setRelationship(EFProviderPresenter::EFRelationship("radarProfileId", &efRadarProfileModel));

   efWellKnownTargets.setRelationship(EFProviderPresenter::EFRelationship("eventId", &efEvents));
   efWellKnownTargets.setRelationship(EFProviderPresenter::EFRelationship("widthTypeId", &efWidthTypes));
   efWellKnownTargets.setRelationship(EFProviderPresenter::EFRelationship("priTypeId", &efPRITypes));
   efWellKnownTargets.setRelationship(EFProviderPresenter::EFRelationship("scanTypeId", &efScanTypes));
   efWellKnownTargets.setRelationship(EFProviderPresenter::EFRelationship("platformId", &efPlatforms));
   efWellKnownTargets.setRelationship(EFProviderPresenter::EFRelationship("radarFunctionId", &efRadarFunctions));
   efWellKnownTargets.setRelationship(EFProviderPresenter::EFRelationship("nameId", &efRadarProfileModel));
   efWellKnownTargets.setRelationship(EFProviderPresenter::EFRelationship("frequencyTypeId", &efFrequencyTypes));
   efWellKnownTargets.setRelationship(EFProviderPresenter::EFRelationship("countryId", &efCountries));
   efWellKnownTargets.setRelationship(EFProviderPresenter::EFRelationship("platformNameId", &efPlatformNames));

   efPlatformNames.setRelationship(EFProviderPresenter::EFRelationship("typeId", &efPlatforms));

   efUserDiaryReports.setRelationship(EFProviderPresenter::EFRelationship("prfTypeId", &efPRITypes));
   efUserDiaryReports.setRelationship(EFProviderPresenter::EFRelationship("pwTypeId", &efWidthTypes));
   efUserDiaryReports.setRelationship(EFProviderPresenter::EFRelationship("rfTypeId", &efFrequencyTypes));
   efUserDiaryReports.setRelationship(EFProviderPresenter::EFRelationship("scanTypeId", &efScanTypes));
   efUserDiaryReports.setRelationship(EFProviderPresenter::EFRelationship("eventId", &efEvents));
   efUserDiaryReports.setRelationship(EFProviderPresenter::EFRelationship("emmiterId", &efRadarFunctions));
   efUserDiaryReports.setRelationship(EFProviderPresenter::EFRelationship("platformId", &efPlatforms));
}


///
/// \brief SunriseDBMS::_checkTableValues
///
void ElephintDBMS::_checkTableValues()
{
   if (efAntennaTypes.count() == 0)
   {
      _initAntennaTypes();
   }
   if (efPRITypes.count() == 0)
   {
      _initPRITypes();
   }
   if (efPlatforms.count() == 0)
   {
      _initPlatforms();
   }
   if (efRadarFunctions.count() == 0)
   {
      _initRadarFunctions();
   }
   if (efRoles.count() == 0)
   {
      _initUserRoles();
   }
   if (efScanTypes.count() == 0)
   {
      _initScanTypes();
   }
   if (efWidthTypes.count() == 0)
   {
      _initWidthTypes();
   }
   if (efUsers.count() == 0)
   {
      _initUsers();
   }
   if (efModulationTypes.count() == 0)
   {
      _initModulations();
   }
   if (efFrequencyTypes.count() == 0)
   {
      _initFrequencyTypes();
   }
   if (efPolarizationTypes.count() == 0)
   {
      _initPolarizations();
   }
   if (efCountries.count() == 0)
   {
      _initCountries();
   }
}


///
/// \brief SunriseDBMS::_initModulations
///
void ElephintDBMS::_initModulations()
{
   QStringList itemList;

   itemList << "Unknown" << "Biphase" << "LFM" << "PolyPhase" << "CoatasCode" << "Simple";
   for (int i = 0; i < itemList.count(); i++)
   {
      ModulationType *m = new ModulationType;
      m->setDisplay(itemList.at(i));
      efModulationTypes.append(m);
   }
   efModulationTypes.saveChanges();
}


///
/// \brief SunriseDBMS::_initFrequencyTypes
///
void ElephintDBMS::_initFrequencyTypes()
{
   QStringList itemList;

   itemList << "Unknown" << "Hopping" << "Agility" << "Diversity" << "Simple";
   for (int i = 0; i < itemList.count(); i++)
   {
      FrequencyType *f = new FrequencyType;
      f->setDisplay(itemList.at(i));
      efFrequencyTypes.append(f);
   }
   efFrequencyTypes.saveChanges();
}


///
/// \brief SunriseDBMS::_initPolarizations
///
void ElephintDBMS::_initPolarizations()
{
   QStringList itemList;

   itemList << "Vertical" << "Horizontal" << "Slant" << "Circular";
   for (int i = 0; i < itemList.count(); i++)
   {
      PolarizationType *p = new PolarizationType;
      p->setDisplay(itemList.at(i));
      efPolarizationTypes.append(p);
   }
   efPolarizationTypes.saveChanges();
}


///
/// \brief SunriseDBMS::_initCountries
///
void ElephintDBMS::_initCountries()
{
   QStringList itemList;
   QStringList dispList;

   itemList << "AND" << "ARE" << "AFG" << "ATG" << "AIA" << "ALB" << "ARM" << "AGO" << "ATA" << "ARG" << "ASM" << "AUT" << "AUS" << "ABW" << "ALA" << "AZE" << "BIH" << "BRB" <<
      "BGD" << "BEL" << "BFA" << "BGR" << "BHR" << "BDI" << "BEN" << "BLM" << "BMU" << "BRN" << "BOL" << "BES" << "BRA" << "BHS" << "BTN" << "BVT" << "BWA" << "BLR" <<
      "BLZ" << "CAN" << "CCK" << "COD" << "CAF" << "COG" << "CHE" << "CIV" << "COK" << "CHL" << "CMR" << "CHN" << "COL" << "CRI" << "CUB" << "CPV" << "CUW" << "CXR" <<
      "CYP" << "CZE" << "DEU" << "DJI" << "DNK" << "DMA" << "DOM" << "DZA" << "ECU" << "EST" << "EGY" << "ESH" << "ERI" << "ESP" << "ETH" << "FIN" << "FJI" << "FLK" <<
      "FSM" << "FRO" << "FRA" << "GAB" << "GBR" << "GRD" << "GEO" << "GUF" << "GGY" << "GHA" << "GIB" << "GRL" << "GMB" << "GIN" << "GLP" << "GNQ" << "GRC" << "SGS" <<
      "GTM" << "GUM" << "GNB" << "GUY" << "HKG" << "HMD" << "HND" << "HRV" << "HTI" << "HUN" << "IDN" << "IRL" << "ISR" << "IMN" << "IND" << "IOT" << "IRQ" << "IRN" <<
      "ISL" << "ITA" << "JEY" << "JAM" << "JOR" << "JPN" << "KEN" << "KGZ" << "KHM" << "KIR" << "COM" << "KNA" << "PRK" << "KOR" << "XKX" << "KWT" << "CYM" << "KAZ" <<
      "LAO" << "LBN" << "LCA" << "LIE" << "LKA" << "LBR" << "LSO" << "LTU" << "LUX" << "LVA" << "LBY" << "MAR" << "MCO" << "MDA" << "MNE" << "MAF" << "MDG" << "MHL" <<
      "MKD" << "MLI" << "MMR" << "MNG" << "MAC" << "MNP" << "MTQ" << "MRT" << "MSR" << "MLT" << "MUS" << "MDV" << "MWI" << "MEX" << "MYS" << "MOZ" << "NAM" << "NCL" <<
      "NER" << "NFK" << "NGA" << "NIC" << "NLD" << "NOR" << "NPL" << "NRU" << "NIU" << "NZL" << "OMN" << "PAN" << "PER" << "PYF" << "PNG" << "PHL" << "PAK" << "POL" <<
      "SPM" << "PCN" << "PRI" << "PSE" << "PRT" << "PLW" << "PRY" << "QAT" << "REU" << "ROU" << "SRB" << "RUS" << "RWA" << "SAU" << "SLB" << "SYC" << "SDN" << "SSD" <<
      "SWE" << "SGP" << "SHN" << "SVN" << "SJM" << "SVK" << "SLE" << "SMR" << "SEN" << "SOM" << "SUR" << "STP" << "SLV" << "SXM" << "SYR" << "SWZ" << "TCA" << "TCD" <<
      "ATF" << "TGO" << "THA" << "TJK" << "TKL" << "TLS" << "TKM" << "TUN" << "TON" << "TUR" << "TTO" << "TUV" << "TWN" << "TZA" << "UKR" << "UGA" << "UMI" << "USA" <<
      "URY" << "UZB" << "VAT" << "VCT" << "VEN" << "VGB" << "VIR" << "VNM" << "VUT" << "WLF" << "WSM" << "YEM" << "MYT" << "ZAF" << "ZMB" << "ZWE" << "SCG" << "ANT";
   dispList << "Andorra" << "United Arab Emirates" << "Afghanistan" << "Antigua and Barbuda" << "Anguilla" << "Albania" << "Armenia" << "Angola" << "Antarctica" <<
      "Argentina" << "American Samoa" << "Austria" << "Australia" << "Aruba" << "Aland Islands" << "Azerbaijan" << "Bosnia and Herzegovina" << "Barbados" <<
      "Bangladesh" << "Belgium" << "Burkina Faso" << "Bulgaria" << "Bahrain" << "Burundi" << "Benin" << "Saint Barthelemy" << "Bermuda" << "Brunei" << "Bolivia" <<
      "Bonaire, Saint Eustatius and Saba " << "Brazil" << "Bahamas" << "Bhutan" << "Bouvet Island" << "Botswana" << "Belarus" << "Belize" << "Canada" << "Cocos Islands" <<
      "Democratic Republic of the Congo" << "Central African Republic" << "Republic of the Congo" << "Switzerland" << "Ivory Coast" << "Cook Islands" << "Chile" <<
      "Cameroon" << "China" << "Colombia" << "Costa Rica" << "Cuba" << "Cabo Verde" << "Curacao" << "Christmas Island" << "Cyprus" << "Czechia" << "Germany" << "Djibouti" <<
      "Denmark" << "Dominica" << "Dominican Republic" << "Algeria" << "Ecuador" << "Estonia" << "Egypt" << "Western Sahara" << "Eritrea" << "Spain" << "Ethiopia" <<
      "Finland" << "Fiji" << "Falkland Islands" << "Micronesia" << "Faroe Islands" << "France" << "Gabon" << "United Kingdom" << "Grenada" << "Georgia" << "French Guiana" <<
      "Guernsey" << "Ghana" << "Gibraltar" << "Greenland" << "Gambia" << "Guinea" << "Guadeloupe" << "Equatorial Guinea" << "Greece" <<
      "South Georgia and the South Sandwich Islands" << "Guatemala" << "Guam" << "Guinea-Bissau" << "Guyana" << "Hong Kong" << "Heard Island and McDonald Islands" <<
      "Honduras" << "Croatia" << "Haiti" << "Hungary" << "Indonesia" << "Ireland" << "Israel" << "Isle of Man" << "India" << "British Indian Ocean Territory" << "Iraq" <<
      "Iran" << "Iceland" << "Italy" << "Jersey" << "Jamaica" << "Jordan" << "Japan" << "Kenya" << "Kyrgyzstan" << "Cambodia" << "Kiribati" << "Comoros" <<
      "Saint Kitts and Nevis" << "North Korea" << "South Korea" << "Kosovo" << "Kuwait" << "Cayman Islands" << "Kazakhstan" << "Laos" << "Lebanon" << "Saint Lucia" <<
      "Liechtenstein" << "Sri Lanka" << "Liberia" << "Lesotho" << "Lithuania" << "Luxembourg" << "Latvia" << "Libya" << "Morocco" << "Monaco" << "Moldova" << "Montenegro" <<
      "Saint Martin" << "Madagascar" << "Marshall Islands" << "North Macedonia" << "Mali" << "Myanmar" << "Mongolia" << "Macao" << "Northern Mariana Islands" <<
      "Martinique" << "Mauritania" << "Montserrat" << "Malta" << "Mauritius" << "Maldives" << "Malawi" << "Mexico" << "Malaysia" << "Mozambique" << "Namibia" << "New Caledonia" <<
      "Niger" << "Norfolk Island" << "Nigeria" << "Nicaragua" << "Netherlands" << "Norway" << "Nepal" << "Nauru" << "Niue" << "New Zealand" << "Oman" << "Panama" << "Peru" << "French Polynesia" <<
      "Papua New Guinea" << "Philippines" << "Pakistan" << "Poland" << "Saint Pierre and Miquelon" << "Pitcairn" << "Puerto Rico" << "Palestinian Territory" << "Portugal" << "Palau" <<
      "Paraguay" << "Qatar" << "Reunion" << "Romania" << "Serbia" << "Russia" << "Rwanda" << "Saudi Arabia" << "Solomon Islands" << "Seychelles" << "Sudan" << "South Sudan" << "Sweden" <<
      "Singapore" << "Saint Helena" << "Slovenia" << "Svalbard and Jan Mayen" << "Slovakia" << "Sierra Leone" << "San Marino" << "Senegal" << "Somalia" <<
      "Suriname" << "Sao Tome and Principe" << "El Salvador" << "Sint Maarten" << "Syria" << "Eswatini" << "Turks and Caicos Islands" << "Chad" <<
      "French Southern Territories" << "Togo" << "Thailand" << "Tajikistan" << "Tokelau" << "Timor Leste" << "Turkmenistan" << "Tunisia" << "Tonga" <<
      "Turkey" << "Trinidad and Tobago" << "Tuvalu" << "Taiwan" << "Tanzania" << "Ukraine" << "Uganda" << "United States Minor Outlying Islands" <<
      "United States" << "Uruguay" << "Uzbekistan" << "Vatican" << "Saint Vincent and the Grenadines" << "Venezuela" << "British Virgin Islands" <<
      "U.S. Virgin Islands" << "Vietnam" << "Vanuatu" << "Wallis and Futuna" << "Samoa" << "Yemen" << "Mayotte" << "South Africa" << "Zambia" <<
      "Zimbabwe" << "Serbia and Montenegro" << "Netherlands Antilles";
   for (int i = 0; i < itemList.count(); i++)
   {
      Countries *c = new Countries;
      c->setName(itemList.at(i));
      c->setDisplay(dispList.at(i));
      efCountries.append(c);
   }
   efCountries.saveChanges();
}


///
/// \brief SunriseDBMS::_initAntennaTypes
///
void ElephintDBMS::_initAntennaTypes()
{
   QStringList itemList;

   itemList << "Unknown" << "Dish" << "Horn" << "Microstrip" << "Wire" << "Array" << "Bicone" << "Discone" << "CauityBackedSpiral";
   for (int i = 0; i < itemList.count(); i++)
   {
      AntennaType *a = new AntennaType;
      a->setDisplay(itemList.at(i));
      efAntennaTypes.append(a);
   }
   efAntennaTypes.saveChanges();
}


///
/// \brief SunriseDBMS::_initPRITypes
///
void ElephintDBMS::_initPRITypes()
{
   QStringList itemList;

   itemList << "Fixed" << "Staggered" << "Jitterd" << "Dwell and Switch" << "Sliding" << "Scheduled" << "Periodic" << "Unknown";
   for (int i = 0; i < itemList.count(); i++)
   {
      PRIType *p = new PRIType;
      p->setDisplay(itemList.at(i));
      efPRITypes.append(p);
   }
   efPRITypes.saveChanges();
}


///
/// \brief SunriseDBMS::_initPlatforms
///
void ElephintDBMS::_initPlatforms()
{
   QStringList itemList;
   QStringList displayList;

   itemList << "A" << "B" << "C" << "E" << "F" << "H" << "K" << "R" << "P" << "W" << "Z" << "V" << "S" << "U" << "AC" << "CR" <<
      "DE" << "FR" << "PC" << "MS" << "CO" << "US" << "BS" << "SU" << "HC" << "OL" << "FA" << "OB" << "CS" << "FP" << "SP" << "UP" << "UA" << "LBR" << "ADS" << "ULB";
   displayList << "Attack aircraft" << "Bomber aircraft" << "Cargo /transport" << "Electronic warfare" << "Fighter aircraft" << "Helicopter" << "Tanker" <<
      "Reconnaissance" << "Maritime surveillance" << "Early warning" << "Aerostat / Balloon" << "Vertical take-off landing/short take off landing" <<
      "Unti-submarine warfare" << "Unmanned aerial vehicle" << "Aircraft carrier " << "Cruiser" << "Destroyer" << "Frigate" << "Patrol craft ship" <<
      "Mine sweeper/hunter " << "Corvette" << "Unknown ship" << "Battle ship" << "Submarine" << "Helicopter carrier" << "Oiler" << "Fast attack craft" <<
      "Observation ship" << "Commercial   ship" << "Fast patrol boat" << "Support ship" << "Unknown platform" << "Unknown aircraft" << "Land based radar" << "Air defense system" << "Unknown land based";
   for (int i = 0; i < itemList.count(); i++)
   {
      Platform *p = new Platform;
      p->setName(itemList.at(i));
      p->setDisplay(displayList.at(i));
      efPlatforms.append(p);
   }
   efPlatforms.saveChanges();
}


///
/// \brief SunriseDBMS::_initRadarFunctions
///
void ElephintDBMS::_initRadarFunctions()
{
   QStringList itemList;
   QStringList displayList;

   itemList << "EW" << "FC" << "NA" << "TT" << "TI" << "MG" << "GI" << "AT" << "HF" << "AE" << "SL" << "BS" << "MS" << "ME" << "CA" << "IF" << "AT"
            << "AT" << "AT" << "AT" << "GA" << "TA" << "RO" << "CS" << "GS" << "GM" << "SS" << "AS" << "WA" << "MT" << "MF" << "AR" << "ZZ";
   displayList << "Early warning radar" << "Fire control radar" << "Navigational radar" << "Target tracking radar" << "Target illumination radar" <<
      "Missile guidance radar" << "Ground controlled intercept radar" << "Air traffic radar" << "Height finder radar" << "Airborne early warning radar" <<
      "Side looking radar" << "Battlefield surveillance radar" << "Maritime surveillance radar" << "Meteorological radar" << "Carrier approach" <<
      "Identification friend or foe" << "Precision approach radar" << "Air route surveillance radar" << "Airport surveillance radar" <<
      "Airport surface detection radar" << "Ground control approach radar" << "Target acquisition radar" << "Range only radar" <<
      "Coastal surveillance radar" << "Ground surveillance radar" << "Ground mapping radar" << "Surface search radar" << "Air search radar" <<
      "Weather avoidance radar" << "Missile tracker radar" << "Multi-function radar" << "Attack radar" << "Unknown";
   for (int i = 0; i < itemList.count(); i++)
   {
      RadarFunction *p = new RadarFunction;
      p->setName(itemList.at(i));
      p->setDisplay(displayList.at(i));
      efRadarFunctions.append(p);
   }
   efRadarFunctions.saveChanges();
}


///
/// \brief SunriseDBMS::_initUserRoles
///
void ElephintDBMS::_initUserRoles()
{
   QStringList itemList;

   itemList << "Admin" << "Factory" << "Super" << "Technical" << "Simple";
   for (int i = 0; i < itemList.count(); i++)
   {
      UserRole *p = new UserRole;
      p->setName(itemList.at(i));
      efRoles.append(p);
   }
   efRoles.saveChanges();
}


///
/// \brief SunriseDBMS::_initScanTypes
///
void ElephintDBMS::_initScanTypes()
{
   QStringList itemList;
   QStringList displayList;

   itemList << "A" << "B" << "C" << "D" << "I" << "F" << "G" << "J" << "K" << "L" << "N" << "O" << "S" << "T" << "U" << "E" << "Z";
   displayList << "Circular scan" << "Horizontal bi-directional sector" << "Vertical bi-directional sector" << "Steady" << "Irregular" << "Conical" << "Lobe switching" << "Raster scan" << "Spiral scan" <<
      "Helical scan" << "Palmer scan (Circular & conical)" << "Palmer (Conical &" << "Vertical uni-directional sector" << "Horizontal uni-directional sector" <<
      "Uni-directional sector (plan undetermined)" << "Electronic" << "Unknown";
   for (int i = 0; i < itemList.count(); i++)
   {
      ScanType *p = new ScanType;
      p->setName(itemList.at(i));
      p->setDisplay(displayList.at(i));
      efScanTypes.append(p);
   }
   efScanTypes.saveChanges();
}


///
/// \brief SunriseDBMS::_initUsers
///
void ElephintDBMS::_initUsers()
{
   MUser *u = new MUser;

   //
   u->setFName("Admin");
   u->setRoleId(1);
   u->setUserName("admin");
   u->setPassword(QCryptographicHash::hash("admin", QCryptographicHash::Sha1).toHex());
   efUsers.append(u);
   //
   u = new MUser;
   u->setFName("user");
   u->setRoleId(5);
   u->setUserName("user");
   u->setPassword(QCryptographicHash::hash("123456", QCryptographicHash::Sha1).toHex());
   efUsers.append(u);
   //
   efUsers.saveChanges();
}


///
/// \brief SunriseDBMS::_initWidthTypes
///
void ElephintDBMS::_initWidthTypes()
{
   QStringList itemList;

   itemList << "Unknown" << "Agile" << "Simple";
   for (int i = 0; i < itemList.count(); i++)
   {
      WidthType *w = new WidthType;
      w->setDisplay(itemList.at(i));
      efWidthTypes.append(w);
   }
   efWidthTypes.saveChanges();
}
