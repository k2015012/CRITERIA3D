#include <QLabel>
#include <QtDebug>
#include <QMessageBox>

#include "project.h"
#include "formInfo.h"
#include "commonConstants.h"
#include "spatialControl.h"
#include "radiationSettings.h"
#include "solarRadiation.h"
#include "interpolationCmd.h"
#include "interpolation.h"
#include "transmissivity.h"
#include "utilities.h"

#include <iostream> //debug

Project::Project()
{
    path = "";
    logFileName = "";
    errorString = "";
    meteoPoints = NULL;
    nrMeteoPoints = 0;
    meteoSettings = new Crit3DMeteoSettings();
    quality = new Crit3DQuality();
    checkSpatialQuality = true;
    currentVariable = noMeteoVar;
    currentFrequency = noFrequency;
    currentDate.setDate(1800,1,1);
    previousDate = currentDate;
    currentHour = 12;
    meteoPointsColorScale = new Crit3DColorScale();
    meteoPointsDbHandler = NULL;
    meteoGridDbHandler = NULL;
    radiationMaps = NULL;

}

void Project::copyInterpolationSettingsToQuality()
{
    //set common properties
    qualityInterpolationSettings.setMinRegressionR2(interpolationSettings.getMinRegressionR2());
    qualityInterpolationSettings.setUseLapseRateCode(interpolationSettings.getUseLapseRateCode());
    qualityInterpolationSettings.setUseThermalInversion(interpolationSettings.getUseThermalInversion());
}

bool Project::readGenericParameters()
{
    //interpolation settings
    interpolationSettings.initialize();
    qualityInterpolationSettings.initialize();

    Q_FOREACH (QString group, settings->childGroups())
    {
        //meteo settings
        if (group == "meteo")
        {
            settings->beginGroup(group);

            if (settings->contains("min_percentage") && !settings->value("min_percentage").toString().isEmpty())
            {
                meteoSettings->setMinimumPercentage(settings->value("min_percentage").toFloat());
            }
            if (settings->contains("prec_threshold") && !settings->value("prec_threshold").toString().isEmpty())
            {
                meteoSettings->setRainfallThreshold(settings->value("prec_threshold").toFloat());
            }
            if (settings->contains("thom_threshold") && !settings->value("thom_threshold").toString().isEmpty())
            {
                meteoSettings->setThomThreshold(settings->value("thom_threshold").toFloat());
            }
            if (settings->contains("samani_coefficient") && !settings->value("samani_coefficient").toString().isEmpty())
            {
                meteoSettings->setTransSamaniCoefficient(settings->value("samani_coefficient").toFloat());
            }

            settings->endGroup();
        }

        //interpolation
        if (group == "interpolation")
        {
            settings->beginGroup(group);

            if (settings->contains("algorithm"))
            {
                std::string algorithm = settings->value("algorithm").toString().toStdString();
                if (interpolationMethodNames.find(algorithm) == interpolationMethodNames.end())
                {
                    errorString = "Unknown interpolation method";
                    return false;
                }
                else
                    interpolationSettings.setInterpolationMethod(interpolationMethodNames.at(algorithm));
            }

            if (settings->contains("gridAggregationMethod"))
            {
                std::string aggrMethod = settings->value("gridAggregationMethod").toString().toStdString();
                if (gridAggregationMethodNames.find(aggrMethod) == gridAggregationMethodNames.end())
                {
                    errorString = "Unknown aggregation method";
                    return false;
                }
                else
                    interpolationSettings.setMeteoGridAggrMethod(gridAggregationMethodNames.at(aggrMethod));
            }

            if (settings->contains("thermalInversion"))
                interpolationSettings.setUseThermalInversion(settings->value("thermalInversion").toBool());

            if (settings->contains("topographicDistance"))
                interpolationSettings.setUseTAD(settings->value("topographicDistance").toBool());

            if (settings->contains("lapseRateCode"))
                interpolationSettings.setUseLapseRateCode(settings->value("lapseRateCode").toBool());

            if (settings->contains("optimalDetrending"))
                interpolationSettings.setUseBestDetrending(settings->value("optimalDetrending").toBool());

            if (settings->contains("minRegressionR2"))
                interpolationSettings.setMinRegressionR2(settings->value("minRegressionR2").toFloat());

            if (settings->contains("useDewPoint"))
                interpolationSettings.setUseDewPoint(settings->value("useDewPoint").toBool());

            settings->endGroup();

            copyInterpolationSettingsToQuality();
        }

        if (group == "quality")
        {
            settings->beginGroup(group);
            if (settings->contains("reference_height") && !settings->value("reference_height").toString().isEmpty())
            {
                //qInfo() << "value: " << settings->value("reference_height");
                quality->setReferenceHeight(settings->value("reference_height").toFloat());
            }
            if (settings->contains("delta_temperature_suspect") && !settings->value("delta_temperature_suspect").toString().isEmpty())
            {
                quality->setDeltaTSuspect(settings->value("delta_temperature_suspect").toFloat());
            }
            if (settings->contains("delta_temperature_wrong") && !settings->value("delta_temperature_wrong").toString().isEmpty())
            {
                quality->setDeltaTWrong(settings->value("delta_temperature_wrong").toFloat());
            }
            if (settings->contains("relhum_tolerance") && !settings->value("relhum_tolerance").toString().isEmpty())
            {
                quality->setRelHumTolerance(settings->value("relhum_tolerance").toFloat());
            }

            settings->endGroup();

        }
    }
    return true;
}


bool Project::readGenericSettings(QString currentPath)
{
    this->path = currentPath;

    QString settingsFileName = currentPath + "settings.ini";

    if (! QFile(settingsFileName).exists())
    {
        logError("Missing settings file: " + settingsFileName);
        return false;
    }

    pathSetting = new QSettings(settingsFileName, QSettings::IniFormat);

    pathSetting->beginGroup("path");
    QString myPath = pathSetting->value("path").toString();
    pathSetting->endGroup();

    if (! myPath.isEmpty())
    {
        if (myPath.right(1) != "/" || myPath.right(1) != "\\" )
        {
            myPath += "/";
        }

        if (myPath.left(1) == ".")
        {
            this->path = currentPath + myPath;
        }
        else
        {
            this->path = myPath;
        }
    }

    pathSetting->beginGroup("location");
    float latitude = pathSetting->value("lat").toFloat();
    float longitude = pathSetting->value("lon").toFloat();
    int utmZone = pathSetting->value("utm_zone").toInt();
    int isUtc = pathSetting->value("is_utc").toBool();
    pathSetting->endGroup();

    if (latitude != 0 && longitude != 0)
    {
        gisSettings.startLocation.latitude = latitude;
        gisSettings.startLocation.longitude = longitude;
        gisSettings.utmZone = utmZone;
        gisSettings.isUTC = isUtc;
    }

    QString parametersFileName = this->path + "DATA/settings/parameters.ini";
    if (! QFile(parametersFileName).exists())
    {
        logError("Missing file: " + parametersFileName);
        return false;
    }
    else
    {
        this->settings = new QSettings(parametersFileName, QSettings::IniFormat);
        return readGenericParameters();
    }

}



/*!
 * \brief loadDEM
 * \param fileName the name of the file
 * \param raster a Crit3DRasterGrid pointer
 * \return true if file is ok, false otherwise
 */
bool Project::loadDEM(QString myFileName)
{
    std::string* myError = new std::string();
    std::string fileName = myFileName.left(myFileName.length()-4).toStdString();

    if (! gis::readEsriGrid(fileName, &DTM, myError))
    {
        qDebug("Load raster failed!");
        return (false);
    }

    setColorScale(noMeteoTerrain, DTM.colorScale);

    // initialize radition maps: slope, aspect, lat/lon
    if (radiationMaps != NULL)
        radiationMaps->clean();
    radiationMaps = new Crit3DRadiationMaps(DTM, gisSettings);

    //reset aggregationPoints meteoGrid
    if (meteoGridDbHandler != NULL)
    {
        meteoGridDbHandler->meteoGrid()->setIsAggregationDefined(false);
        // TODO
        // ciclo sulle celle della meteo grid -> clean vettore aggregation points
    }

    //set interpolation settings DEM
    interpolationSettings.setCurrentDEM(&DTM);
    qualityInterpolationSettings.setCurrentDEM(&DTM);

    //check points position with respect to DEM
    checkMeteoPointsDEM();

    return (true);
}


bool Project::checkProxySetting(QString group, std::string* name, std::string* grdName,
                                std::string* table, std::string* field, bool* isActive)
{
    std::string grid;

    *name = group.right(group.size()-6).toStdString();
    settings->beginGroup(group);
    grid = settings->value("raster").toString().toStdString();
    *grdName = path.toStdString() + grid;
    *table = settings->value("table").toString().toStdString();
    *field = settings->value("field").toString().toStdString();
    *isActive = settings->value("active").toBool();
    settings->endGroup();

    return (*name != "" && (grid != "" || (*table != "" && *field != "")));
}

bool Project::addProxy(std::string name_, std::string gridName_, bool isActive,
                       std::string table_, std::string field_)
{
    Crit3DProxy myProxy;

    myProxy.setName(name_);
    myProxy.setGridName(gridName_);

    return true;
}

bool Project::readProxies()
{
    std::string proxyName, proxyGridName, proxyTable, proxyField;
    bool isActive;
    int proxyNr = 0;
    bool isGridLoaded;

    interpolationSettings.initializeProxy();
    meteoPointsDbHandler->initializeProxy();

    Q_FOREACH (QString group, settings->childGroups())
    {
        //proxy variables (for interpolation)
        if (group.startsWith("proxy"))
        {
            isGridLoaded = false;

            if (! checkProxySetting(group, &proxyName, &proxyGridName, &proxyTable, &proxyField, &isActive))
            {
                errorString = "error parsing proxy " + proxyName;
                return false;
            }

            Crit3DProxy myProxy;

            myProxy.setName(proxyName);
            myProxy.setGridName(proxyGridName);

            if (myProxy.getProxyPragaName() == height)
            {
                if (DTM.isLoaded)
                {
                    isGridLoaded = true;
                    myProxy.setGrid(&DTM);
                }
                else
                    isGridLoaded = loadProxyGrid(&myProxy);
            }
            else
                isGridLoaded = loadProxyGrid(&myProxy);

            if ( !isGridLoaded && (proxyTable == "" || proxyField == ""))
            {
                errorString = "error reading grid, table or field for proxy " + proxyName;
                return false;
            }

            if (myProxy.getProxyPragaName() == height)
            {
                interpolationSettings.setIndexHeight(proxyNr);
                // quality spatial control use only elevation
                qualityInterpolationSettings.addProxy(myProxy, isActive);
                qualityInterpolationSettings.setIndexHeight(0);
            }

            interpolationSettings.addProxy(myProxy, isActive);

            if (isGridLoaded && meteoPointsDbHandler != NULL)
            {
                meteoPointsDbHandler->addProxy(myProxy, proxyTable, proxyField);
                proxyNr++;
            }
        }
    }

    return true;
}


bool Project::getMeteoPointSelected(int i)
{
    if (meteoPointsSelected.isEmpty()) return true;

    for (int j = 0; j < meteoPointsSelected.size(); j++)
    {
        if (meteoPoints[i].latitude == meteoPointsSelected[j].latitude && meteoPoints[i].longitude == meteoPointsSelected[j].longitude)
            return true;
    }

    return false;
}




void Project::setCurrentDate(QDate myDate)
{
    if (myDate != this->currentDate)
    {
        this->previousDate = this->currentDate;
        this->currentDate = myDate;
    }
}

void Project::setCurrentHour(int myHour)
{
    this->currentHour = myHour;
}

void Project::setFrequency(frequencyType myFrequency)
{
    this->currentFrequency = myFrequency;
}

QDate Project::getCurrentDate()
{
    return this->currentDate;
}

Crit3DTime Project::getCurrentTime()
{
    return getCrit3DTime(this->currentDate, this->currentHour);
}

int Project::getCurrentHour()
{
    return this->currentHour;
}

frequencyType Project::getFrequency()
{
    return this->currentFrequency;
}

meteoVariable Project::getCurrentVariable()
{
    return this->currentVariable;
}


bool Project::loadlastMeteoData()
{
    QDate lastDateD = meteoPointsDbHandler->getLastDay(daily).date();
    QDate lastDateH = meteoPointsDbHandler->getLastDay(hourly).date();

    /*
     *
    */
/*
    QDate firstDateD = meteoPointsDbHandler->getFirstDay(daily).date();
    QDate firstDateH = meteoPointsDbHandler->getFirstDay(hourly).date();


    int numberOfDaysD = firstDateD.daysTo(lastDateD) + 1;
    int numberOfDaysH = firstDateH.daysTo(lastDateH) + 1;
    int myHourlyFraction = 1;

    for (int i=0; i < nrMeteoPoints; i++)
    {
        meteoPoints[i].initializeObsDataD(numberOfDaysD, getCrit3DDate(firstDateD));
        meteoPoints[i].initializeObsDataH(myHourlyFraction, numberOfDaysH, getCrit3DDate(firstDateH));
    }
*/
    /*
     *
    */

    QDate lastDate = (lastDateD > lastDateH) ? lastDateD : lastDateH;

    this->setCurrentDate(lastDate);
    this->setCurrentHour(12);

    return this->loadMeteoPointsData (lastDate, lastDate, true);
}


bool Project::updateMeteoPointsData()
{
    return loadMeteoPointsData(currentDate, currentDate, true);
}


bool Project::loadMeteoPointsData(QDate firstDate, QDate lastDate, bool showInfo)
{
    //check
    if (firstDate == QDate(1800,1,1) || lastDate == QDate(1800,1,1)) return false;

    bool isData = false;
    FormInfo myInfo;
    int step;

    QString infoStr = "Load data: " + firstDate.toString();

    if (firstDate != lastDate)
        infoStr += " - " + lastDate.toString();

    if (showInfo)
        step = myInfo.start(infoStr, nrMeteoPoints);

    for (int i=0; i < nrMeteoPoints; i++)
    {

        if (showInfo)
            if ((i % step) == 0) myInfo.setValue(i);

        if (meteoPointsDbHandler->loadDailyData(getCrit3DDate(firstDate), getCrit3DDate(lastDate), &(meteoPoints[i])))
        {
            isData = true;
        }
        if (meteoPointsDbHandler->loadHourlyData(getCrit3DDate(firstDate), getCrit3DDate(lastDate), &(meteoPoints[i])))
        {
            isData = true;
        }

    }

    if (showInfo) myInfo.close();

    return isData;
}


void Project::getMeteoPointsRange(float *minimum, float *maximum)
{
    *minimum = NODATA;
    *maximum = NODATA;

    if (currentFrequency == noFrequency || currentVariable == noMeteoVar)
        return;

    float v;
    for (int i = 0; i < nrMeteoPoints; i++)
    {
        v = meteoPoints[i].currentValue;

        if (v != NODATA && meteoPoints[i].quality == quality::accepted)
        {
            if (*minimum == NODATA)
            {
                *minimum = v;
                *maximum = v;
            }
            else if (v < *minimum) *minimum = v;
            else if (v > *maximum) *maximum = v;
        }
    }
}

void Project::closeMeteoPointsDB()
{

    if (meteoPointsDbHandler != NULL)
    {
        delete meteoPointsDbHandler;
    }

    if (meteoPoints != NULL)
    {
        delete [] meteoPoints;
    }

    meteoPointsDbHandler = NULL;
    meteoPoints = NULL;

    meteoPointsSelected.clear();
    nrMeteoPoints = 0;
}

void Project::closeMeteoGridDB()
{

    if (meteoGridDbHandler != NULL)
    {
        delete meteoGridDbHandler;
    }

    meteoGridDbHandler = NULL;

}


bool Project::loadMeteoPointsDB(QString dbName)
{
    if (dbName == "") return false;

    closeMeteoPointsDB();

    meteoPointsDbHandler = new Crit3DMeteoPointsDbHandler(dbName);
    if (meteoPointsDbHandler->error != "")
    {
        logError(meteoPointsDbHandler->error);
        closeMeteoPointsDB();
        return false;
    }

    QList<Crit3DMeteoPoint> listMeteoPoints = meteoPointsDbHandler->getPropertiesFromDb(this->gisSettings);

    nrMeteoPoints = listMeteoPoints.size();
    if (nrMeteoPoints == 0)
    {
        logError("Missing meteo point properties.");
        closeMeteoPointsDB();
        return false;
    }

    meteoPoints = new Crit3DMeteoPoint[nrMeteoPoints];

    for (int i=0; i < nrMeteoPoints; i++)
    {
        meteoPoints[i] = listMeteoPoints[i];
    }

    listMeteoPoints.clear();

    //interpolation proxies
    if (! readProxies())
    {
        logError("Error loading interpolation proxies");
        return false;
    }
    readProxyValues();

    //position with respect to DEM
    if (DTM.isLoaded)
        checkMeteoPointsDEM();

    return true;
}


bool Project::loadMeteoGridDB(QString xmlName)
{

    if (xmlName == "")
        return false;

    this->meteoGridDbHandler = new Crit3DMeteoGridDbHandler();
    this->meteoGridDbHandler->meteoGrid()->setGisSettings(this->gisSettings);


    if (! this->meteoGridDbHandler->parseXMLGrid(xmlName, &errorString))
        return false;

    if (! this->meteoGridDbHandler->openDatabase(&errorString))
        return false;

    if (! this->meteoGridDbHandler->loadCellProperties(&errorString))
        return false;

    if (! this->meteoGridDbHandler->updateGridDate(&errorString))
        return false;




// test
//    this->meteoGridDbHandler->saveCellCurrrentGridDaily(&errorString, "lauraD", QDate(1985,01,01), 231, 30);
//    this->meteoGridDbHandler->saveCellCurrrentGridDaily(&errorString, "lauraD", QDate(1985,01,01), 231, NODATA);
//    this->meteoGridDbHandler->saveCellCurrrentGridDaily(&errorString, "lauraD", QDate(1985,01,02), 231, 40);
//    this->meteoGridDbHandler->saveCellCurrentGridDailyFF(&errorString, "lauraD", QDate(1985,01,01), "DAILY_TAVG", 30);
//    this->meteoGridDbHandler->saveCellCurrentGridDailyFF(&errorString, "lauraD", QDate(1985,01,01), "DAILY_W_INT_AVG", 10);
//    this->meteoGridDbHandler->saveCellCurrentGridDailyFF(&errorString, "lauraD", QDate(1985,01,01), "DAILY_W_INT_AVG", 20);
//    this->meteoGridDbHandler->saveCellCurrentGridDailyFF(&errorString, "lauraD", QDate(1985,01,02), "DAILY_TAVG", NODATA);
//    this->meteoGridDbHandler->saveCellCurrentGridDailyFF(&errorString, "lauraD", QDate(1985,01,03), "DAILY_TAVG", 40);
//    this->meteoGridDbHandler->saveCellCurrentGridHourlyFF(&errorString, "lauraH", QDateTime(QDate(1991,01,01),QTime(9,0,0)), "TAVG", 30);
//    this->meteoGridDbHandler->saveCellCurrentGridHourlyFF(&errorString, "lauraH", QDateTime(QDate(1991,01,01),QTime(9,0,0)), "TAVG", NODATA);
//    this->meteoGridDbHandler->saveCellCurrentGridHourlyFF(&errorString, "lauraH", QDateTime(QDate(1991,01,01),QTime(10,0,0)), "TAVG", 20);

//    this->meteoGridDbHandler->saveCellCurrentGridHourly(&errorString, "lauraH", QDateTime(QDate(1991,01,01),QTime(9,0,0)), 78, 30);
//    this->meteoGridDbHandler->saveCellCurrentGridHourly(&errorString, "lauraH", QDateTime(QDate(1991,01,01),QTime(9,0,0)), 78, NODATA);
//    this->meteoGridDbHandler->saveCellCurrentGridHourly(&errorString, "lauraH", QDateTime(QDate(1991,01,01),QTime(10,0,0)), 78, 20);

//    QDateTime firstDateDB;
//    std::vector<float> hourlyVarList = this->meteoGridDbHandler->loadGridHourlyVar(&errorString, "01019", precipitation, QDateTime(QDate(1991,01,01),QTime(9,0,0)), QDateTime(QDate(1991,01,2),QTime(9,0,0)), &firstDateDB);
//    if (hourlyVarList.size() == 0)
//        return false;

    /*
    QDate lastDate = this->meteoGridDbHandler->lastDate();
    if (! this->loadMeteoGridDailyData(lastDate, lastDate) )
        return false;
*/

    //test
//    QDate firstDateDB;
//    std::vector<float> dailyVarList = this->meteoGridDbHandler->loadGridDailyVar(&errorString, "01010", dailyPrecipitation, QDate(1991,01,01), QDate(1991,01,10), &firstDateDB);
//    std::vector<float> dailyVarList = this->meteoGridDbHandler->loadGridDailyVarFixedFields(&errorString, "01011", dailyAirTemperatureMin, QDate(2018,04,15), QDate(2018,04,17), &firstDateDB);
//    if (dailyVarList.size() == 0)
//        return false;

//    if (! this->meteoGridDbHandler->loadGridHourlyData(&errorString, "01019", QDateTime(QDate(1991,01,01),QTime(9,0,0)), QDateTime(QDate(1991,01,2),QTime(9,0,0))))
//        return false;
//    QDateTime firstDateDB;
//    std::vector<float> hourlyVarList = this->meteoGridDbHandler->loadGridHourlyVarFixedFields(&errorString, "01019", airTemperature, QDateTime(QDate(2018,04,15),QTime(20,0,0)), QDateTime(QDate(2018,04,15),QTime(23,0,0)), &firstDateDB);
//        if (hourlyVarList.size() == 0)
//            return false;

//    loadMeteoGridData(QDate(2018,04,29), QDate(2018,04,30), 1);
//    QList<meteoVariable> meteoVariableList = { dailyAirTemperatureMin , dailyAirTemperatureMax , dailyAirTemperatureAvg , dailyAirRelHumidityMax };
//    this->meteoGridDbHandler->saveCellGridDailyData(&errorString, "write00097", 23, 2, QDate(2018,04,29), QDate(2018,04,30), meteoVariableList);


//    loadMeteoGridData(QDate(2018,04,17), QDate(2018,04,30), 1);
//    QList<meteoVariable> meteoVariableList = { airTemperature , precipitation , airHumidity , globalIrradiance };
//    this->meteoGridDbHandler->saveCellGridHourlyData(&errorString, "Hwrite00097", 23, 2, QDateTime(QDate(2018,04,29),QTime(9,0,0)), QDateTime(QDate(2018,04,30),QTime(9,0,0)), meteoVariableList);
//    this->meteoGridDbHandler->saveCellGridDailyDataFF(&errorString, "DW00097", 23, 2, QDate(2018,04,29), QDate(2018,04,30));
//    this->meteoGridDbHandler->saveCellGridHourlyDataFF(&errorString, "HW00097", 23, 2, QDateTime(QDate(2018,04,17),QTime(20,0,0)), QDateTime(QDate(2018,04,18),QTime(9,0,0)));
    return true;
}


void Project::loadMeteoGridData(QDate firstDate, QDate lastDate, bool showInfo)
{
    if (this->meteoGridDbHandler != NULL)
    {
        QDateTime t1 = QDateTime(firstDate);
        QDateTime t2 = QDateTime(lastDate.addDays(1));

        this->loadMeteoGridDailyData(firstDate, lastDate, showInfo);

        this->loadMeteoGridHourlyData(t1, t2, showInfo);
    }
}


bool Project::loadMeteoGridDailyData(QDate firstDate, QDate lastDate, bool showInfo)
{
    std::string id;
    int count = 0;

    FormInfo myInfo;
    int infoStep;

    if (showInfo)
    {
        QString infoStr = "Load grid daily data: " + firstDate.toString();
        if (firstDate != lastDate) infoStr += " - " + lastDate.toString();
        infoStep = myInfo.start(infoStr, this->meteoGridDbHandler->gridStructure().header().nrRows);
    }

    for (int row = 0; row < this->meteoGridDbHandler->gridStructure().header().nrRows; row++)
    {
        if (showInfo && (row % infoStep) == 0)
            myInfo.setValue(row);

        for (int col = 0; col < this->meteoGridDbHandler->gridStructure().header().nrCols; col++)
        {
            if (this->meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id))
            {
                if (!this->meteoGridDbHandler->gridStructure().isFixedFields())
                {
                    if (this->meteoGridDbHandler->loadGridDailyData(&errorString, QString::fromStdString(id), firstDate, lastDate))
                    {
                        count = count + 1;
                    }
                }
                else
                {
                    if (this->meteoGridDbHandler->loadGridDailyDataFixedFields(&errorString, QString::fromStdString(id), firstDate, lastDate))
                    {
                        count = count + 1;
                    }
                }
            }
        }
    }

    if (showInfo) myInfo.close();

    if (count == 0)
    {
        errorString = "No Data Available";
        return false;
    }
    else
        return true;
}


bool Project::loadMeteoGridHourlyData(QDateTime firstDate, QDateTime lastDate, bool showInfo)
{
    std::string id;
    int count = 0;
    FormInfo myInfo;
    int infoStep;

    if (showInfo)
    {
        QString infoStr = "Load grid hourly data: " + firstDate.toString("yyyy-MM-dd:hh") + " - " + lastDate.toString("yyyy-MM-dd:hh");
        infoStep = myInfo.start(infoStr, this->meteoGridDbHandler->gridStructure().header().nrRows);
    }

    for (int row = 0; row < this->meteoGridDbHandler->gridStructure().header().nrRows; row++)
    {
        if (showInfo && (row % infoStep) == 0)
            myInfo.setValue(row);

        for (int col = 0; col < this->meteoGridDbHandler->gridStructure().header().nrCols; col++)
        {
            if (this->meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id))
            {
                if (!this->meteoGridDbHandler->gridStructure().isFixedFields())
                {
                    if (this->meteoGridDbHandler->loadGridHourlyData(&errorString, QString::fromStdString(id), firstDate, lastDate))
                    {
                        count = count + 1;
                    }
                }
                else
                {
                    if (this->meteoGridDbHandler->loadGridHourlyDataFixedFields(&errorString, QString::fromStdString(id), firstDate, lastDate))
                    {
                        count = count + 1;
                    }
                }
            }
        }
    }

    if (showInfo) myInfo.close();

    if (count == 0)
    {
        errorString = "No Data Available";
        return false;
    }
    else
        return true;
}

void Project::checkMeteoPointsDEM()
{
    for (int i=0; i < nrMeteoPoints; i++)
        meteoPoints[i].isInsideDem = ! gis::isOutOfGridXY(meteoPoints[i].point.utm.x, meteoPoints[i].point.utm.y, DTM.header);
}

void Project::readProxyValues()
{
    if (meteoPointsDbHandler == NULL)
        return;

    for (int i = 0; i < nrMeteoPoints; i++)
        meteoPointsDbHandler->readPointProxyValues(&meteoPoints[i]);
}

bool Project::writeTopographicDistanceMaps()
{
    if (nrMeteoPoints == 0)
    {
        errorString = "No meteo points available";
        return false;
    }

    if (! DTM.isLoaded)
    {
        errorString = "No DEM loaded";
        return false;
    }

    QString mapsFolder = this->path + "DATA/GEO/TAD/";
    if (! QDir(mapsFolder).exists())
        QDir().mkdir(mapsFolder);

    FormInfo myInfo;
    int infoStep;
    QString infoStr = "Computing topographic distance maps...";
    infoStep = myInfo.start(infoStr, nrMeteoPoints);

    std::string myError;
    std::string fileName;
    gis::Crit3DRasterGrid myMap;
    for (int i=0; i < nrMeteoPoints; i++)
    {
        myInfo.setValue(i);

        if (meteoPoints[i].active)
        {
            if (gis::topographicDistanceMap(meteoPoints[i].point, DTM, &myMap))
            {
                fileName = mapsFolder.toStdString() + "TAD_" + meteoPoints[i].id;
                if (! gis::writeEsriGrid(fileName, &myMap, &myError))
                {
                    errorString = myError;
                    return false;
                }
            }
        }
    }

    myInfo.close();

    return true;
}

bool Project::loadTopographicDistanceMaps()
{
    if (nrMeteoPoints == 0)
    {
        errorString = "No meteo points available";
        return false;
    }

    QString mapsFolder = this->path + "DATA/GEO/TAD/";
    if (! QDir(mapsFolder).exists()) return false;

    FormInfo myInfo;
    int infoStep;
    QString infoStr = "Loading topographic distance maps...";
    infoStep = myInfo.start(infoStr, nrMeteoPoints);

    std::string myError;
    std::string fileName;


    for (int i=0; i < nrMeteoPoints; i++)
    {
        myInfo.setValue(i);

        if (meteoPoints[i].active)
        {
            fileName = mapsFolder.toStdString() + "TAD_" + meteoPoints[i].id;
            meteoPoints[i].topographicDistance = new gis::Crit3DRasterGrid();
            if (! gis::readEsriGrid(fileName, meteoPoints[i].topographicDistance, &myError))
            {
                errorString = myError;
                return false;
            }
        }
    }

    myInfo.close();

    return true;
}

bool Project::interpolationDem(meteoVariable myVar, const Crit3DTime& myTime, gis::Crit3DRasterGrid *myRaster, bool showInfo)
{
    std::vector <Crit3DInterpolationDataPoint> interpolationPoints;

    // check quality and pass data to interpolation
    if (!checkAndPassDataToInterpolation(quality, myVar, meteoPoints, nrMeteoPoints, myTime,
                                         &qualityInterpolationSettings, &interpolationSettings, interpolationPoints,
                                         checkSpatialQuality))
    {
        errorString = "No data available";
        return false;
    }

    // Proxy vars regression and detrend
    if (showInfo)
    {
        FormInfo myInfo;
        myInfo.start("Preparing interpolation...", 0);
    }

    if (! preInterpolation(interpolationPoints, &interpolationSettings, meteoPoints, nrMeteoPoints, myVar, myTime))
    {
        errorString = "Interpolation: error in function preInterpolation";
        return false;
    }

    // Interpolate
    if (! interpolationRaster(interpolationPoints, &interpolationSettings, myRaster, DTM, myVar, showInfo))
    {
       errorString = "Interpolation: error in function interpolateGridDtm";
        return false;
    }

    Crit3DTime t = myTime;
    myRaster->timeString = t.toStdString();
    return true;
}

bool Project::interpolateDemRadiation(const Crit3DTime& myTime, gis::Crit3DRasterGrid *myRaster, bool showInfo)
{
    std::vector <Crit3DInterpolationDataPoint> interpolationPoints;

    radSettings.setGisSettings(&gisSettings);

    gis::Crit3DPoint mapCenter = DTM.mapCenter();

    int intervalWidth = radiation::estimateTransmissivityWindow(&radSettings, DTM, *radiationMaps, &mapCenter, myTime, int(HOUR_SECONDS));

    // almost a meteoPoint with transmissivity data
    if (! computeTransmissivity(&radSettings, meteoPoints, nrMeteoPoints, intervalWidth, myTime, DTM))
        if (! computeTransmissivityFromTRange(meteoPoints, nrMeteoPoints, myTime))
        {
            errorString = "Function interpolateRasterRadiation: it is not possible to compute transmissivity.";
            return false;
        }

    if (! checkAndPassDataToInterpolation(quality, atmTransmissivity, meteoPoints, nrMeteoPoints,
                                        myTime, &qualityInterpolationSettings,
                                        &interpolationSettings, interpolationPoints, checkSpatialQuality))
    {
        errorString = "Function interpolateRasterRadiation: not enough transmissivity data.";
        return false;
    }

    preInterpolation(interpolationPoints, &interpolationSettings, meteoPoints, nrMeteoPoints, atmTransmissivity, myTime);

    if (! interpolationRaster(interpolationPoints, &interpolationSettings, this->radiationMaps->transmissivityMap, DTM, atmTransmissivity, showInfo))
    {
        errorString = "Function interpolateRasterRadiation: error interpolating transmissivity.";
        return false;
    }

    if (! radiation::computeRadiationGridPresentTime(&radSettings, this->DTM, this->radiationMaps, myTime))
    {
        errorString = "Function interpolateRasterRadiation: error computing solar radiation";
        return false;
    }

    myRaster->copyGrid(*(this->radiationMaps->globalRadiationMap));

    return true;
}

bool Project::interpolationDemMain(meteoVariable myVar, const Crit3DTime& myTime, gis::Crit3DRasterGrid *myRaster, bool showInfo)
{
    if (myVar == noMeteoVar)
    {
        errorString = "No variable selected";
        return false;
    }

    if (! DTM.isLoaded)
    {
        errorString = "No DEM loaded";
        return false;
    }

    if (nrMeteoPoints == 0)
    {
        errorString = "No points available";
        return false;
    }

    myRaster->initializeGrid(this->DTM);

    if (myVar == globalIrradiance)
    {
        Crit3DTime measureTime = myTime.addSeconds(-1800);
        return interpolateDemRadiation(measureTime, myRaster, showInfo);
    }
    else
    {
        return interpolationDem(myVar, myTime, myRaster, showInfo);
    }
}

float Project::meteoDataConsistency(meteoVariable myVar, const Crit3DTime& timeIni, const Crit3DTime& timeFin)
{
    float dataConsistency = 0.0;
    for (int i = 0; i < nrMeteoPoints; i++)
        dataConsistency = maxValue(dataConsistency, meteoPoints[i].obsDataConsistencyH(myVar, timeIni, timeFin));

    return dataConsistency;
}


/*-------------------

   LOG functions

 -------------------*/

bool Project::setLogFile(QString callingProgram)
{
    if (!QDir(this->path + "log").exists())
         QDir().mkdir(this->path + "log");

    QString myDate = QDateTime().currentDateTime().toString("yyyy-MM-dd hh.mm");
    QString fileName = myDate + callingProgram + ".txt";

    this->logFileName = this->path + "log\\" + fileName;

    this->logFile.open(this->logFileName.toStdString().c_str());
    return (this->logFile.is_open());
}

void Project::log(std::string myStr)
{
    if (logFile.is_open())
        logFile << myStr << std::endl;
}

void Project::logError(QString myStr)
{
    errorString = myStr.toStdString();
    logError();
}

void Project::logError()
{
    if (logFile.is_open())
        logFile << "----ERROR!----\n" << this->errorString << std::endl;
    else
        QMessageBox::critical(NULL, "Error!", QString::fromStdString(errorString));
}


