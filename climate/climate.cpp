#include <QString>
#include <QDate>

#include <math.h>       /* ceil */

#include "commonConstants.h"
#include "climate.h"
#include "crit3dDate.h"
#include "utilities.h"
#include "statistics.h"
#include "quality.h"
#include "dbClimate.h"

bool elaborationOnPoint(std::string *myError, Crit3DMeteoPointsDbHandler* meteoPointsDbHandler, Crit3DMeteoGridDbHandler* meteoGridDbHandler,
    Crit3DMeteoPoint* meteoPointTemp, Crit3DClimate* clima, bool isMeteoGrid, QDate startDate, QDate endDate, bool isAnomaly, Crit3DMeteoSettings* meteoSettings)
{

    float percValue;
    float result;

    std::vector<float> outputValues;

    meteoComputation elab1MeteoComp;
    meteoComputation elab2MeteoComp;
    try
    {
        elab1MeteoComp = MapMeteoComputation.at(clima->elab1().toStdString());
    }
    catch (const std::out_of_range& )
    {
        elab1MeteoComp = noMeteoComp;
    }

    if (clima->elab2() == "")
    {
        elab2MeteoComp = noMeteoComp;
    }
    else
    {
        try
        {
            elab2MeteoComp = MapMeteoComputation.at(clima->elab2().toStdString());
        }
        catch (const std::out_of_range& )
        {
            elab2MeteoComp = noMeteoComp;
        }
    }

    if (clima->param1IsClimate())
    {
        QList<float> paramList;
        QString table = getTable(clima->param1ClimateField());
        paramList = readElab(clima->db(), table, myError, QString::fromStdString(meteoPointTemp->id), clima->param1ClimateField());
        if (clima->getParam1ClimateIndex() != NODATA && clima->getParam1ClimateIndex() <= paramList.size())
        {
            clima->setParam1(paramList.at( clima->getParam1ClimateIndex() - 1 ));
        }
        else
        {
            clima->setParam1(NODATA);
        }
    }

    bool dataLoaded = preElaboration(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPointTemp, isMeteoGrid, clima->variable(), elab1MeteoComp, startDate, endDate, outputValues, &percValue, meteoSettings, clima->getElabSettings());

    if (dataLoaded)
    {
        // check
        Crit3DDate startD(startDate.day(), startDate.month(), clima->yearStart());
        Crit3DDate endD(endDate.day(), endDate.month(), clima->yearStart());

        if ( clima->nYears() < 0)
        {
            startD.year = clima->yearStart() + clima->nYears();
        }
        else if ( clima->nYears() > 0)
        {
            endD.year = clima->yearStart() + clima->nYears();
        }

        if (difference (startD, endD) < 0)
        {
            *myError = "Wrong dates!";
            return false;
        }
        if (clima->elab1() == "")
        {
            *myError = "Missing elaboration";
            return false;
        }

        result = computeStatistic(outputValues, meteoPointTemp, clima, startD, endD, clima->nYears(), elab1MeteoComp, elab2MeteoComp, meteoSettings);

        if (isAnomaly)
        {
            return anomalyOnPoint(meteoPointTemp, result);
        }
        else
        {
            meteoPointTemp->elaboration = result;
            if (meteoPointTemp->elaboration != NODATA)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    else if (isAnomaly)
    {
        meteoPointTemp->anomaly = NODATA;
        meteoPointTemp->anomalyPercentage = NODATA;
    }

    return false;

}

bool anomalyOnPoint(Crit3DMeteoPoint* meteoPoint, float refValue)
{

    bool anomalyOnPoint = (refValue != NODATA && meteoPoint->elaboration != NODATA);

    if (anomalyOnPoint)
    {
        meteoPoint->anomaly = meteoPoint->elaboration - refValue;
        if (refValue != 0)
        {
            meteoPoint->anomalyPercentage = (meteoPoint->elaboration - refValue) / abs(refValue) * 100;
        }
        else
        {
            meteoPoint->anomalyPercentage = NODATA;
        }
    }
    else
    {
        meteoPoint->anomaly = NODATA;
        meteoPoint->anomalyPercentage = NODATA;
    }
    return anomalyOnPoint;

}

bool passingClimateToAnomaly(std::string *myError, Crit3DMeteoPoint* meteoPointTemp, Crit3DClimate* clima, Crit3DMeteoPoint* meteoPoints, int nrMeteoPoints, Crit3DElaborationSettings *elabSettings)
{

    QList<float> valueList;
    QString table = getTable(clima->climateElab());
    int index = clima->getParam1ClimateIndex();
    valueList = readElab(clima->db(), table, myError, QString::fromStdString(meteoPointTemp->id), clima->climateElab());
    if (index != NODATA && index <= valueList.size())
    {
        // MP found
        return anomalyOnPoint(meteoPointTemp, valueList.at( index - 1 ));
    }
    else
    {
        float maxVerticalDist = elabSettings->getAnomalyPtsMaxDeltaZ();
        float maxHorizontalDist = elabSettings->getAnomalyPtsMaxDistance();

        QList<QString> idList;
        if (table == "climate_generic" || table == "climate_annual")
        {
            idList = getIdListFromElab(clima->db(), table, myError, clima->climateElab());
        }
        else
        {
            idList = getIdListFromElab(clima->db(), table, myError, clima->climateElab(), index);
        }


        float minDist = NODATA;
        float currentDist = NODATA;
        bool noHeigth = false;
        QString idNearMP = "";

        for (int i = 0; i < idList.size(); i++)
        {
            for (int j = 0; j < nrMeteoPoints; j++)
            {
                if ( QString::fromStdString(meteoPoints[j].id) == idList.at(i))
                {
                    currentDist = gis::computeDistance(meteoPointTemp->point.utm.x, meteoPointTemp->point.utm.y, meteoPoints[j].point.utm.x, meteoPoints[j].point.utm.y);
                    if (currentDist < maxHorizontalDist)
                    {
                        if (minDist == NODATA || currentDist < minDist)
                        {
                            if (meteoPointTemp->point.z == NODATA && meteoPoints[j].point.z == NODATA)
                            {
                                noHeigth = true;
                            }
                            if (noHeigth || abs(meteoPointTemp->point.z - meteoPoints[j].point.z) < maxVerticalDist)
                            {
                                minDist = currentDist;
                                idNearMP = QString::fromStdString(meteoPoints[j].id);
                            }

                        }
                    }
                }
            }
        }

        if (minDist != NODATA)
        {
            valueList.clear();
            valueList = readElab(clima->db(), table, myError, idNearMP, clima->climateElab());
            if (index != NODATA && index <= valueList.size())
            {
                return anomalyOnPoint(meteoPointTemp, valueList.at( index - 1 ));
            }
            else
            {
                return anomalyOnPoint(meteoPointTemp, NODATA);
            }
        }
    }

}

bool climateOnPoint(std::string *myError, Crit3DMeteoPointsDbHandler* meteoPointsDbHandler, Crit3DMeteoGridDbHandler* meteoGridDbHandler,
                    Crit3DClimate* clima, Crit3DMeteoPoint* meteoPointTemp, std::vector<float> &outputValues, bool isMeteoGrid, QDate startDate, QDate endDate, bool changeDataSet, Crit3DMeteoSettings* meteoSettings)
{

    float percValue;
    bool dataLoaded = true;

    if (isMeteoGrid)
    {
        clima->setDb(meteoGridDbHandler->db());
    }
    else
    {
        clima->setDb(meteoPointsDbHandler->getDb());
    }


    meteoComputation elab1MeteoComp;
    meteoComputation elab2MeteoComp;
    try
    {
        elab1MeteoComp = MapMeteoComputation.at(clima->elab1().toStdString());
    }
    catch (const std::out_of_range& )
    {
        elab1MeteoComp = noMeteoComp;
    }
    if (clima->elab2() == "")
    {
        elab2MeteoComp = noMeteoComp;
    }
    else
    {
        try
        {
            elab2MeteoComp = MapMeteoComputation.at(clima->elab2().toStdString());
        }
        catch (const std::out_of_range& )
        {
            elab2MeteoComp = noMeteoComp;
        }
    }

    // check id points
    if (changeDataSet == false)
    {
        // check download data
        if ( (clima->variable() != clima->getCurrentVar() || clima->yearStart() < clima->getCurrentYearStart() || clima->yearEnd() > clima->getCurrentYearEnd()) ||
                (clima->elab1() != clima->getCurrentElab1() && (elab1MeteoComp == correctedDegreeDaysSum || elab1MeteoComp == huglin || elab1MeteoComp == winkler ||  elab1MeteoComp == fregoni) ) )
        {
            changeDataSet = true;
        }
    }


    if (changeDataSet)
    {
        clima->setCurrentVar(clima->variable());
        clima->setCurrentElab1(clima->elab1());
        clima->setCurrentYearStart(clima->yearStart());
        clima->setCurrentYearEnd(clima->yearEnd());

        outputValues.clear();
        meteoPointTemp->nrObsDataDaysD = 0;
        meteoPointTemp->nrObsDataDaysH = 0;

        dataLoaded = preElaboration(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPointTemp, isMeteoGrid, clima->variable(), elab1MeteoComp, startDate, endDate, outputValues, &percValue, meteoSettings, clima->getElabSettings());
    }

    if (dataLoaded)
    {
        if (climateTemporalCycle(myError, clima, outputValues, meteoPointTemp, elab1MeteoComp, elab2MeteoComp, meteoSettings))
        {
            return true;
        }
    }

    return false;
}

bool climateTemporalCycle(std::string *myError, Crit3DClimate* clima, std::vector<float> &outputValues, Crit3DMeteoPoint* meteoPoint, meteoComputation elab1, meteoComputation elab2, Crit3DMeteoSettings* meteoSettings)
{

    QSqlDatabase db = clima->db();

    float result;

    QList<float> paramList;
    if (clima->param1IsClimate())
    {
        QString table = getTable(clima->param1ClimateField());
        paramList = readElab(db, table, myError, QString::fromStdString(meteoPoint->id), clima->param1ClimateField());
    }

    switch(clima->periodType())
    {

    case dailyPeriod:
    {

        clima->setCurrentPeriodType(clima->periodType());

        bool okAtLeastOne = false;
        int nLeapYears = 0;
        int totYears = 0;
        int nDays = 366;
        int leapYear;
        std::vector<float> allResults;

        Crit3DDate startD;
        Crit3DDate endD;

        for (int i = clima->yearStart(); i<=clima->yearEnd(); i++)
        {
            if (isLeapYear(i))
            {
                nLeapYears = nLeapYears + 1;
                leapYear = i;
            }
            totYears = totYears + 1;
        }

        float minPerc = meteoSettings->getMinimumPercentage();

        if (nLeapYears == 0)
        {
            nDays = nDays - 1;
        }

        for (int i = 1; i<=nDays; i++)
        {
            if (nLeapYears == 0)
            {
                startD = getDateFromDoy(clima->yearStart(), i);
            }
            else
            {
                startD = getDateFromDoy(leapYear, i);
            }
            endD = startD;

            if (i == 366)
            {
                meteoSettings->setMinimumPercentage(minPerc * nLeapYears/totYears);
            }

            if (clima->param1IsClimate())
            {
                int climateIndex = getClimateIndexFromElab(getQDate(startD), clima->param1ClimateField());
                if (climateIndex != NODATA && climateIndex <= paramList.size())
                {
                    clima->setParam1(paramList.at( climateIndex - 1 ));
                }
                else
                {
                    clima->setParam1(NODATA);
                }
            }

            result = computeStatistic(outputValues, meteoPoint, clima, startD, endD, clima->nYears(), elab1, elab2, meteoSettings);

            if (result != NODATA)
            {
                okAtLeastOne = true;
            }
            allResults.push_back(result);
        }

        // if there are no leap years, save NODATA into 366row
        if (nLeapYears == 0)
        {
            allResults.push_back(NODATA);
        }

        meteoSettings->setMinimumPercentage(minPerc);

        // reset currentPeriod
        clima->setCurrentPeriodType(noPeriodType);

        if (okAtLeastOne)
        {
            return saveDailyElab(db, myError, QString::fromStdString(meteoPoint->id), allResults, clima->climateElab());
        }
        else
        {
            return false;
        }

    }
    case decadalPeriod:
    {
        bool okAtLeastOne = false;
        std::vector<float> allResults;

        QList<float> paramList;
        if (clima->param1IsClimate())
        {
            QString table = getTable(clima->param1ClimateField());
            paramList = readElab(db, table, myError, QString::fromStdString(meteoPoint->id), clima->param1ClimateField());
        }

        for (int i = 1; i<=36; i++)
        {
            int dayStart;
            int dayEnd;
            int month;

            intervalDecade(i, clima->yearStart(), &dayStart, &dayEnd, &month);

            Crit3DDate startD (dayStart, month, clima->yearStart());
            Crit3DDate endD (dayEnd, month, clima->yearStart());

            if (clima->param1IsClimate())
            {
                int climateIndex = getClimateIndexFromElab(getQDate(startD), clima->param1ClimateField());
                if (climateIndex != NODATA && climateIndex <= paramList.size())
                {
                    clima->setParam1(paramList.at( climateIndex - 1 ));
                }
                else
                {
                    clima->setParam1(NODATA);
                }
            }

            result = computeStatistic(outputValues, meteoPoint, clima, startD, endD, clima->nYears(), elab1, elab2, meteoSettings);

            if (result != NODATA)
            {
                okAtLeastOne = true;
            }
            allResults.push_back(result);
        }
        if (okAtLeastOne)
        {
            return saveDecadalElab(db, myError, QString::fromStdString(meteoPoint->id), allResults, clima->climateElab());
        }
        else
        {
            return false;
        }
    }

    case monthlyPeriod:
    {
        bool okAtLeastOne = false;
        std::vector<float> allResults;

        QList<float> paramList;
        if (clima->param1IsClimate())
        {
            QString table = getTable(clima->param1ClimateField());
            paramList = readElab(db, table, myError, QString::fromStdString(meteoPoint->id), clima->param1ClimateField());
        }

        for (int i = 1; i<=12; i++)
        {

            Crit3DDate startD (1, i, clima->yearStart());
            QDate temp(clima->yearEnd(), i, 1);
            int dayEnd = temp.daysInMonth();

            Crit3DDate endD (dayEnd, i, clima->yearStart());

            if (clima->param1IsClimate())
            {
                int climateIndex = getClimateIndexFromElab(getQDate(startD), clima->param1ClimateField());
                if (climateIndex != NODATA && climateIndex <= paramList.size())
                {
                    clima->setParam1(paramList.at( climateIndex - 1 ));
                }
                else
                {
                    clima->setParam1(NODATA);
                }
            }

            result = computeStatistic(outputValues, meteoPoint, clima, startD, endD, clima->nYears(), elab1, elab2, meteoSettings);

            if (result != NODATA)
            {
                okAtLeastOne = true;
            }
            allResults.push_back(result);
        }
        if (okAtLeastOne)
        {
            return saveMonthlyElab(db, myError, QString::fromStdString(meteoPoint->id), allResults, clima->climateElab());
        }
        else
        {
            return false;
        }
    }

    case seasonalPeriod:
    {
        bool okAtLeastOne = false;
        std::vector<float> allResults;

        QList<float> paramList;
        if (clima->param1IsClimate())
        {
            QString table = getTable(clima->param1ClimateField());
            paramList = readElab(db, table, myError, QString::fromStdString(meteoPoint->id), clima->param1ClimateField());
        }

        for (int i = 1; i<=4; i++)
        {

            int monthEnd;
            int dayEnd;
            int seasonalNPeriodYears ;

            if (i<4)
            {
                monthEnd = i*3+2;
                seasonalNPeriodYears = 0;
            }
            else
            {
                monthEnd = 2;
                seasonalNPeriodYears = -1;
            }

            QDate temp(clima->yearEnd(), monthEnd, 1);

            dayEnd = temp.daysInMonth();

            Crit3DDate startD (1, i*3, clima->yearStart());
            Crit3DDate endD (dayEnd, monthEnd, clima->yearEnd());

            if (clima->param1IsClimate())
            {
                int climateIndex = getClimateIndexFromElab(getQDate(startD), clima->param1ClimateField());
                if (climateIndex != NODATA && climateIndex <= paramList.size())
                {
                    clima->setParam1(paramList.at( climateIndex - 1 ));
                }
                else
                {
                    clima->setParam1(NODATA);
                }
            }

            result = computeStatistic(outputValues, meteoPoint, clima, startD, endD, seasonalNPeriodYears, elab1, elab2, meteoSettings);

            if (result != NODATA)
            {
                okAtLeastOne = true;
            }
            allResults.push_back(result);
        }
        if (okAtLeastOne)
        {
            return saveSeasonalElab(db, myError, QString::fromStdString(meteoPoint->id), allResults, clima->climateElab());
        }
        else
        {
            return false;
        }

    }

    case annualPeriod:
    {

        Crit3DDate startD (1, 1, clima->yearStart());
        Crit3DDate endD (31, 12, clima->yearStart());

        if (clima->param1IsClimate())
        {
            QList<float> paramList;
            QString table = getTable(clima->param1ClimateField());
            paramList = readElab(db, table, myError, QString::fromStdString(meteoPoint->id), clima->param1ClimateField());

            int climateIndex = getClimateIndexFromElab(getQDate(startD), clima->param1ClimateField());
            if (climateIndex != NODATA && climateIndex <= paramList.size())
            {
                clima->setParam1(paramList.at( climateIndex - 1 ));
            }
            else
            {
                clima->setParam1(NODATA);
            }
        }

        result = computeStatistic(outputValues, meteoPoint, clima, startD, endD, clima->nYears(), elab1, elab2, meteoSettings);

        if (result != NODATA)
        {
            return saveAnnualElab(db, myError, QString::fromStdString(meteoPoint->id), result, clima->climateElab());
        }
        else
        {
            return false;
        }
    }

    case genericPeriod:
    {

        Crit3DDate startD = getCrit3DDate(clima->genericPeriodDateStart());
        Crit3DDate endD = getCrit3DDate(clima->genericPeriodDateEnd());

        if (clima->param1IsClimate())
        {
            QList<float> paramList;
            QString table = getTable(clima->param1ClimateField());
            paramList = readElab(db, table, myError, QString::fromStdString(meteoPoint->id), clima->param1ClimateField());

            int climateIndex = getClimateIndexFromElab(getQDate(startD), clima->param1ClimateField());
            if (climateIndex != NODATA && climateIndex <= paramList.size())
            {
                clima->setParam1(paramList.at( climateIndex - 1 ));
            }
            else
            {
                clima->setParam1(NODATA);
            }
        }

        result = computeStatistic(outputValues, meteoPoint, clima, startD, endD, clima->nYears(), elab1, elab2, meteoSettings);

        if (result != NODATA)
        {
            return saveGenericElab(db, myError, QString::fromStdString(meteoPoint->id), result, clima->climateElab());
        }
        else
        {
            return false;
        }
    }

    default:
        return false;

    }
}


float loadDailyVarSeries(std::string *myError, Crit3DMeteoPointsDbHandler *meteoPointsDbHandler,
        Crit3DMeteoGridDbHandler *meteoGridDbHandler, Crit3DMeteoPoint* meteoPoint, bool isMeteoGrid,
        meteoVariable variable, QDate first, QDate last)
{

    std::vector<float> dailyValues;
    QDate firstDateDB;
    Crit3DQuality qualityCheck;
    int nrValidValues = 0;
    int nrRequestedValues = first.daysTo(last) +1;

    // meteoGrid
    if (isMeteoGrid)
    {
        if (meteoGridDbHandler->gridStructure().isFixedFields())
        {
            dailyValues = meteoGridDbHandler->loadGridDailyVarFixedFields(myError, QString::fromStdString(meteoPoint->id), variable, first, last, &firstDateDB);
        }
        else
        {
            dailyValues = meteoGridDbHandler->loadGridDailyVar(myError, QString::fromStdString(meteoPoint->id), variable, first, last, &firstDateDB);
        }
    }
    // meteoPoint
    else
    {
        dailyValues = meteoPointsDbHandler->loadDailyVar(myError, variable, getCrit3DDate(first), getCrit3DDate(last), &firstDateDB, meteoPoint );
    }


    if ( dailyValues.empty() )
    {
        return 0;
    }
    else
    {
        if (meteoPoint->nrObsDataDaysD == 0)
        {
            meteoPoint->initializeObsDataD(int(dailyValues.size()), getCrit3DDate(firstDateDB));
        }

        Crit3DDate currentDate = getCrit3DDate(firstDateDB);
        for (unsigned int i = 0; i < dailyValues.size(); i++)
        {
            quality::qualityType qualityT = qualityCheck.syntacticQualitySingleValue(variable, dailyValues[i]);
            if (qualityT == quality::accepted)
            {
                nrValidValues = nrValidValues + 1;
            }
            meteoPoint->setMeteoPointValueD(currentDate, variable, dailyValues[i]);
            currentDate = currentDate.addDays(1);
        }

        float percValue = float(nrValidValues) / float(nrRequestedValues);
        return percValue;
    }
}


float loadDailyVarSeries_SaveOutput(std::string *myError, Crit3DMeteoPointsDbHandler *meteoPointsDbHandler,
        Crit3DMeteoGridDbHandler *meteoGridDbHandler, Crit3DMeteoPoint* meteoPoint, bool isMeteoGrid,
        meteoVariable variable, QDate first, QDate last, std::vector<float> &outputValues)
{
    std::vector<float> dailyValues;
    QDate firstDateDB;
    Crit3DQuality qualityCheck;
    int nrValidValues = 0;
    int nrRequestedValues = first.daysTo(last) +1;

    // meteoGrid
    if (isMeteoGrid)
    {
        if (meteoGridDbHandler->gridStructure().isFixedFields())
        {
            dailyValues = meteoGridDbHandler->loadGridDailyVarFixedFields(myError, QString::fromStdString(meteoPoint->id), variable, first, last, &firstDateDB);
        }
        else
        {
            dailyValues = meteoGridDbHandler->loadGridDailyVar(myError, QString::fromStdString(meteoPoint->id), variable, first, last, &firstDateDB);
        }

    }
    // meteoPoint
    else
    {
        dailyValues = meteoPointsDbHandler->loadDailyVar(myError, variable, getCrit3DDate(first), getCrit3DDate(last), &firstDateDB, meteoPoint );
    }


    if ( dailyValues.empty() )
    {
        return 0;
    }
    else
    {
        if (meteoPoint->nrObsDataDaysD == 0)
        {
            meteoPoint->initializeObsDataD(int(dailyValues.size()), getCrit3DDate(firstDateDB));
        }

        Crit3DDate currentDate = getCrit3DDate(firstDateDB);
        for (unsigned int i = 0; i < dailyValues.size(); i++)
        {
            quality::qualityType qualityT = qualityCheck.syntacticQualitySingleValue(variable, dailyValues[i]);
            if (qualityT == quality::accepted)
            {
                nrValidValues = nrValidValues + 1;
            }
            meteoPoint->setMeteoPointValueD(currentDate, variable, dailyValues[i]);

            outputValues.push_back(dailyValues[i]);

            currentDate = currentDate.addDays(1);

        }

        float percValue = float(nrValidValues) / float(nrRequestedValues);
        return percValue;
    }
}


float loadHourlyVarSeries(std::string *myError, Crit3DMeteoPointsDbHandler* meteoPointsDbHandler,
           Crit3DMeteoGridDbHandler* meteoGridDbHandler, Crit3DMeteoPoint* meteoPoint, bool isMeteoGrid,
           meteoVariable variable, QDateTime first, QDateTime last)
{
    std::vector<float> hourlyValues;
    QDateTime firstDateDB;
    Crit3DQuality qualityCheck;
    int nrValidValues = 0;
    int nrRequestedDays = first.daysTo(last) +1;
    int nrRequestedValues = nrRequestedDays * 24 * meteoPoint->hourlyFraction;

    // meteoGrid
    if (isMeteoGrid)
    {
        if (meteoGridDbHandler->gridStructure().isFixedFields())
        {
            hourlyValues = meteoGridDbHandler->loadGridHourlyVarFixedFields(myError, QString::fromStdString(meteoPoint->id), variable, first, last, &firstDateDB);
        }
        else
        {
            hourlyValues = meteoGridDbHandler->loadGridHourlyVar(myError, QString::fromStdString(meteoPoint->id), variable, first, last, &firstDateDB);
        }
    }
    // meteoPoint
    else
    {
        hourlyValues = meteoPointsDbHandler->loadHourlyVar(myError, variable, getCrit3DDate(first.date()), getCrit3DDate(last.date()), &firstDateDB, meteoPoint );
    }


    if ( hourlyValues.empty() )
    {
        return 0;
    }
    else
    {
        if (meteoPoint->nrObsDataDaysH == 0)
        {
            int nrOfDays = ceil(float(hourlyValues.size()) / float(24 * meteoPoint->hourlyFraction));
            meteoPoint->initializeObsDataH(meteoPoint->hourlyFraction, nrOfDays,getCrit3DDate(firstDateDB.date()));
        }

        for (unsigned int i = 0; i < hourlyValues.size(); i++)
        {
            quality::qualityType qualityT = qualityCheck.syntacticQualitySingleValue(variable, hourlyValues[i]);
            if (qualityT == quality::accepted)
            {
                nrValidValues = nrValidValues + 1;
            }

            meteoPoint->setMeteoPointValueH(Crit3DDate(firstDateDB.date().day(), firstDateDB.date().month(), firstDateDB.date().year()), firstDateDB.time().hour(), firstDateDB.time().minute(), variable, hourlyValues[i]);

            firstDateDB = firstDateDB.addSecs(3600);
        }

        float percValue = float(nrValidValues) / float(nrRequestedValues);
        return percValue;
    }

}


// compute daily Thom using Tmax e RHmin
float thomDayTime(float tempMax, float relHumMinAir)
{

    Crit3DQuality qualityCheck;
    quality::qualityType qualityT = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMax, tempMax);
    quality::qualityType qualityRelHumMinAir = qualityCheck.syntacticQualitySingleValue(dailyAirRelHumidityMin, relHumMinAir);


    // TODO nella versione vb ammessi anche i qualitySuspectData, questo tipo per ora non è stato implementato
    if ( qualityT == quality::accepted && qualityRelHumMinAir == quality::accepted )
    {
            return computeThomIndex(tempMax, relHumMinAir);
    }
    else
        return NODATA;

}

// compute daily Thom using Tmin e RHmax
float thomNightTime(float tempMin, float relHumMaxAir)
{

    Crit3DQuality qualityCheck;
    quality::qualityType qualityT = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMin, tempMin);
    quality::qualityType qualityRelHumMaxAir = qualityCheck.syntacticQualitySingleValue(dailyAirRelHumidityMax, relHumMaxAir);

    // TODO nella versione vb ammessi anche i qualitySuspectData, questo tipo per ora non è stato implementato
    if ( qualityT == quality::accepted && qualityRelHumMaxAir == quality::accepted )
    {
            return computeThomIndex(tempMin, relHumMaxAir);
    }
    else
        return NODATA;

}

// compuote hourly thom
float thomH(float tempAvg, float relHumAvgAir)
{

    Crit3DQuality qualityCheck;
    quality::qualityType qualityT = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureAvg, tempAvg);
    quality::qualityType qualityRelHumAvgAir = qualityCheck.syntacticQualitySingleValue(dailyAirRelHumidityAvg, relHumAvgAir);

    // TODO nella versione vb ammessi anche i qualitySuspectData, questo tipo per ora non è stato implementato
    if ( qualityT == quality::accepted && qualityRelHumAvgAir == quality::accepted )
    {
            return computeThomIndex(tempAvg, relHumAvgAir);
    }
    else
        return NODATA;
}


// compute # hours thom >  threshold per day
int thomDailyNHoursAbove(float* tempAvg, float* relHumAvgAir, float thomthreshold, float minimumPercentage)
{

    int nData = 0;
    int thomDailyNHoursAbove = NODATA;
    for (int hour = 0; hour < 24; hour++)
    {
        float thom = thomH(tempAvg[hour], relHumAvgAir[hour]);
        if (thom != NODATA)
        {
            nData = nData + 1;
            if (thomDailyNHoursAbove == NODATA)
                thomDailyNHoursAbove = 0;
            if (thom > thomthreshold)
                thomDailyNHoursAbove = thomDailyNHoursAbove + 1;
        }
    }
    if ( (float(nData) / 24.f * 100.f) < minimumPercentage)
        thomDailyNHoursAbove = NODATA;

    return thomDailyNHoursAbove;


}

// compute daily max thom value
float thomDailyMax(float *tempAvg, float* relHumAvgAir, float minimumPercentage)
{
    int nData = 0;
    int thomDailyMax = NODATA;
    for (int hour = 0; hour < 24; hour++)
    {
        float thom = thomH(tempAvg[hour], relHumAvgAir[hour]);
        if (thom != NODATA)
        {
            nData = nData + 1;
            if (thom > thomDailyMax)
                thomDailyMax = thom;
        }
    }
    if ( (float(nData) / 24.f * 100.f) < minimumPercentage)
        thomDailyMax = NODATA;

    return thomDailyMax;
}

// compute daily avg thom value
float thomDailyMean(float *tempAvg, float* relHumAvgAir, float minimumPercentage)
{

    int nData = 0;
    std::vector<float> thomValues;
    float thomDailyMean;

    for (int hour = 1; hour <= 24; hour++)
    {
        float thom = thomH(tempAvg[hour], relHumAvgAir[hour]);
        if (thom != NODATA)
        {
            thomValues.push_back(thom);
            nData = nData + 1;
        }
    }
    if ( (float(nData) / 24.f * 100.f) < minimumPercentage)
        thomDailyMean = NODATA;
    else
        thomDailyMean = statistics::mean(thomValues, nData);


    return thomDailyMean;

}

float dailyLeafWetnessComputation(int *leafW, float minimumPercentage)
{

    int nData = 0;
    float dailyLeafWetnessRes = 0;

    for (int hour = 0; hour < 24; hour++)
    {
        if (leafW[hour] == 0 || leafW[hour] == 1)
        {
                dailyLeafWetnessRes = dailyLeafWetnessRes + leafW[hour];
                nData = nData + 1;
        }
    }
    if ( (float(nData) / 24.f * 100.f) < minimumPercentage)
        dailyLeafWetnessRes = NODATA;

    return dailyLeafWetnessRes;

}

float computeDailyBIC(float prec, float etp)
{

    Crit3DQuality qualityCheck;

    // TODO nella versione vb ammessi anche i qualitySuspectData, questo tipo per ora non è stato implementato
    quality::qualityType qualityPrec = qualityCheck.syntacticQualitySingleValue(dailyPrecipitation, prec);
    quality::qualityType qualityETP = qualityCheck.syntacticQualitySingleValue(dailyReferenceEvapotranspirationHS, etp);
    if (qualityPrec == quality::accepted && qualityETP == quality::accepted)
    {
            return (prec - etp);
    }
    else
        return NODATA;

}

float dailyThermalRange(float Tmin, float Tmax)
{

    Crit3DQuality qualityCheck;

    // TODO nella versione vb ammessi anche i qualitySuspectData, questo tipo per ora non è stato implementato
    quality::qualityType qualityTmin = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMin, Tmin);
    quality::qualityType qualityTmax = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMax, Tmax);
    if (qualityTmin  == quality::accepted && qualityTmax == quality::accepted)
        return (Tmax - Tmin);
    else
        return NODATA;

}

float dailyAverageT(float Tmin, float Tmax)
{

        Crit3DQuality qualityCheck;

        // TODO nella versione vb ammessi anche i qualitySuspectData, questo tipo per ora non è stato implementato
        quality::qualityType qualityTmin = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMin, Tmin);
        quality::qualityType qualityTmax = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMax, Tmax);
        if (qualityTmin  == quality::accepted && qualityTmax == quality::accepted)
            return ( (Tmin + Tmax) / 2) ;
        else
            return NODATA;

}

float dailyEtpHargreaves(float Tmin, float Tmax, Crit3DDate date, double latitude)
{

    Crit3DQuality qualityCheck;

    // TODO nella versione vb ammessi anche i qualitySuspectData, questo tipo per ora non è stato implementato
    quality::qualityType qualityTmin = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMin, Tmin);
    quality::qualityType qualityTmax = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMax, Tmax);
    int dayOfYear = getDoyFromDate(date);
    if (qualityTmin  == quality::accepted && qualityTmax == quality::accepted)
        return ET0_Hargreaves(TRANSMISSIVITY_SAMANI_COEFF_DEFAULT, latitude, dayOfYear, Tmax, Tmin);
    else
        return NODATA;

}

float dewPoint(float relHumAir, float tempAir)
{

    if (relHumAir == NODATA || relHumAir == 0 || tempAir == NODATA)
        return NODATA;

    relHumAir = minValue(100, relHumAir);

    float saturatedVaporPres = exp((16.78 * tempAir - 116.9) / (tempAir + 237.3));
    float actualVaporPres = relHumAir / 100 * saturatedVaporPres;
    return (log(actualVaporPres) * 237.3 + 116.9) / (16.78 - log(actualVaporPres));

}


float computeWinkler(Crit3DMeteoPoint* meteoPoint, Crit3DDate firstDate, Crit3DDate finishDate, float minimumPercentage)
{

    float computeWinkler = 0;

    Crit3DQuality qualityCheck;
    int index;
    int count = 0;
    bool checkData;
    float Tavg;


    int numberOfDays = difference(firstDate, finishDate) +1;

    Crit3DDate presentDate = firstDate;
    for (int i = 0; i < numberOfDays; i++)
    {
        index = difference(meteoPoint->obsDataD[0].date, presentDate);
        checkData = false;
        if (index < meteoPoint->nrObsDataDaysD)
        {

            // TO DO nella versione vb il check prevede anche l'immissione del parametro height
            quality::qualityType qualityTavg = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureAvg, meteoPoint->obsDataD[index].tAvg);
            if (qualityTavg == quality::accepted)
            {
                Tavg = meteoPoint->obsDataD[index].tAvg;
                checkData = true;
            }
            else
            {
                quality::qualityType qualityTmin = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMin, meteoPoint->obsDataD[index].tMin);
                quality::qualityType qualityTmax = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMax, meteoPoint->obsDataD[index].tMax);
                if (qualityTmin  == quality::accepted && qualityTmax == quality::accepted)
                {
                    Tavg = (meteoPoint->obsDataD[index].tMin + meteoPoint->obsDataD[index].tMax)/2;
                    checkData = true;
                }

            }

        }
        if (checkData)
        {
            if (Tavg > WINKLERTHRESHOLD)
            {
                Tavg = Tavg - WINKLERTHRESHOLD;
            }
            else
            {
                Tavg = 0;
            }
            computeWinkler = computeWinkler + Tavg;
            count = count + 1;
        }
        presentDate = presentDate.addDays(1);
    }
    if (numberOfDays != 0)
    {
        if ( (float(count) / float(numberOfDays) * 100.f) < minimumPercentage )
        {
            computeWinkler = NODATA;
        }
    }

    return computeWinkler;

}

float computeLastDayBelowThreshold(std::vector<float> &inputValues, Crit3DDate firstDateDailyVar, Crit3DDate firstDate, Crit3DDate finishDate, float param1)
{
    unsigned int index;
    float lastDay = NODATA;

    int numberOfDays = difference(firstDate, finishDate) +1;
    Crit3DDate presentDate = finishDate;
    for (int i = 0; i < numberOfDays; i++)
    {
        index = difference(firstDateDailyVar, presentDate);
        if ( index < inputValues.size())
        {
            if (inputValues.at(index) != NODATA && inputValues.at(index) < param1)
            {
                lastDay = getDoyFromDate(presentDate);
            }
        }
        presentDate = presentDate.addDays(-1);
    }


    return lastDay;
}

float computeHuglin(Crit3DMeteoPoint* meteoPoint, Crit3DDate firstDate, Crit3DDate finishDate, float minimumPercentage)
{
    float computeHuglin = 0;

    const int threshold = 10;                   // LC vanno lasciate qui queste costanti?
    const float K = 1.04f;                      //coeff. K di Huglin lunghezza giorno (=1.04 per ER)

    Crit3DQuality qualityCheck;
    int index;
    int count = 0;
    bool checkData;
    float Tavg;
    float Tmax;


    int numberOfDays = difference(firstDate, finishDate) +1;

    Crit3DDate presentDate = firstDate;
    for (int i = 0; i < numberOfDays; i++)
    {
        index = difference(meteoPoint->obsDataD[0].date, presentDate);
        checkData = false;
        if ( index < meteoPoint->nrObsDataDaysD)
        {

            // TO DO nella versione vb il check prevede anche l'immissione del parametro height
            quality::qualityType qualityTavg = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureAvg, meteoPoint->obsDataD[index].tAvg);
            quality::qualityType qualityTmax = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMax, meteoPoint->obsDataD[index].tMax);
            if (qualityTavg == quality::accepted && qualityTmax == quality::accepted)
            {
                Tmax = meteoPoint->obsDataD[index].tMax;
                Tavg = meteoPoint->obsDataD[index].tAvg;
                checkData = true;
            }
            else
            {
                quality::qualityType qualityTmin = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMin, meteoPoint->obsDataD[index].tMin);
                if (qualityTmin  == quality::accepted && qualityTmax == quality::accepted)
                {
                    Tmax = meteoPoint->obsDataD[index].tMax;
                    Tavg = (meteoPoint->obsDataD[index].tMin + Tmax)/2;
                    checkData = true;
                }

            }

        }
        if (checkData)
        {
            computeHuglin = computeHuglin + K * ((Tavg - threshold) + (Tmax - threshold)) / 2;
            count = count + 1;
        }
        presentDate = presentDate.addDays(1);
    }
    if (numberOfDays != 0)
    {
        if ( (float(count) / float(numberOfDays) * 100.f) < minimumPercentage )
        {
            computeHuglin = NODATA;
        }
    }

    return computeHuglin;
}


float computeFregoni(Crit3DMeteoPoint* meteoPoint, Crit3DDate firstDate, Crit3DDate finishDate, float minimumPercentage)
{
    const int threshold = 10;
    float computeFregoni = 0;

    Crit3DQuality qualityCheck;
    int index;
    int count = 0;
    int myDaysBelow = 0;
    bool checkData;
    float tMin, tMax;
    float tRange;
    float sumTRange = 0;


    int numberOfDays = difference(firstDate, finishDate) +1;

    Crit3DDate presentDate = firstDate;
    for (int i = 0; i < numberOfDays; i++)
    {
        index = difference(meteoPoint->obsDataD[0].date, presentDate);
        checkData = false;
        if ( index < meteoPoint->nrObsDataDaysD)
        {

            // TO DO nella versione vb il check prevede anche l'immissione del parametro height
            quality::qualityType qualityTmin = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMin, meteoPoint->obsDataD[index].tMin);
            quality::qualityType qualityTmax = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMax, meteoPoint->obsDataD[index].tMax);
            if (qualityTmin == quality::accepted && qualityTmax == quality::accepted)
            {
                tMin = meteoPoint->obsDataD[index].tMin;
                tMax = meteoPoint->obsDataD[index].tMax;
                tRange = tMax - tMin;
                checkData = true;
            }

        }
        if (checkData)
        {
            sumTRange = sumTRange + tRange;
            if (tMin < threshold)
            {
                myDaysBelow = myDaysBelow + 1;
            }
            count = count + 1;
        }
        presentDate = presentDate.addDays(1);
    }
    if (numberOfDays != 0)
    {
        if ( (float(count) / float(numberOfDays) * 100.f) < minimumPercentage )
        {
            computeFregoni = NODATA;
        }
        else
        {
            computeFregoni = sumTRange * myDaysBelow;
        }
    }

    return computeFregoni;
}


float computeCorrectedSum(Crit3DMeteoPoint* meteoPoint, Crit3DDate firstDate, Crit3DDate finishDate, float param, float minimumPercentage)
{
    float computeCorrectedSum;

    Crit3DQuality qualityCheck;
    int index;
    int count = 0;
    bool checkData;
    float tMin, tMax, tAvg;
    float numTmp, numerator, denominator;


    int numberOfDays = difference(firstDate, finishDate) +1;

    Crit3DDate presentDate = firstDate;
    for (int i = 0; i < numberOfDays; i++)
    {
        index = difference(meteoPoint->obsDataD[0].date, presentDate);
        checkData = false;
        if ( index < meteoPoint->nrObsDataDaysD)
        {

            // TO DO nella versione vb il check prevede anche l'immissione del parametro height
            quality::qualityType qualityTmin = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMin, meteoPoint->obsDataD[index].tMin);
            quality::qualityType qualityTmax = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureMax, meteoPoint->obsDataD[index].tMax);
            if (qualityTmin == quality::accepted && qualityTmax == quality::accepted)
            {
                tMax = meteoPoint->obsDataD[index].tMax;
                tMin = meteoPoint->obsDataD[index].tMin;
                checkData = true;
            }

        }
        if (checkData)
        {
            if (param < tMax)
            {
                if (param <= tMin)
                {
                    tAvg = (tMax + tMin) / 2;
                    computeCorrectedSum = computeCorrectedSum + (tAvg - param);
                }
                else
                {
                    numTmp = tMax - param;
                    numerator = numTmp * numTmp;
                    denominator = 2 * (tMax - tMin);
                    if (denominator != 0)
                    {
                        computeCorrectedSum = computeCorrectedSum + numerator/denominator;
                    }
                }
            }
            count = count + 1;
        }
        presentDate = presentDate.addDays(1);
    }
    if (numberOfDays != 0)
    {
        if ( (float(count) / float(numberOfDays) * 100.f) < minimumPercentage )
        {
            computeCorrectedSum = NODATA;
        }
    }


    return computeCorrectedSum;
}


bool elaborateDailyAggregatedVar(meteoVariable myVar, Crit3DMeteoPoint meteoPoint, std::vector<float> &outputValues, float* percValue, Crit3DMeteoSettings* meteoSettings)
{
    outputValues.clear();

    frequencyType aggregationFrequency = getAggregationFrequency(myVar);

    if (aggregationFrequency == hourly)
    {
            return elaborateDailyAggregatedVarFromHourly(myVar, meteoPoint, outputValues, meteoSettings);
    }
    else if (aggregationFrequency == daily)
    {
            return elaborateDailyAggregatedVarFromDaily(myVar, meteoPoint, outputValues, percValue);
    }
    else
        return false;

}


frequencyType getAggregationFrequency(meteoVariable myVar)
{

    if (myVar == dailyThomHoursAbove || myVar == dailyThomAvg || myVar == dailyThomMax || myVar == dailyLeafWetness)
    {
        return hourly;
    }
    else if (myVar != NODATA)
    {
        return daily;
    }
    else
    {
        return noFrequency;
    }

}

bool elaborateDailyAggregatedVarFromDaily(meteoVariable myVar, Crit3DMeteoPoint meteoPoint, std::vector<float> &outputValues, float* percValue)
{

    float res;
    int nrValidValues = 0;
    Crit3DDate date = meteoPoint.obsDataD[0].date;
    Crit3DQuality qualityCheck;

    for (int index = 0; index < meteoPoint.nrObsDataDaysD; index++)
    {
        switch(myVar)
        {
        case dailyThomDaytime:
                res = thomDayTime(meteoPoint.obsDataD[index].tMax, meteoPoint.obsDataD[index].rhMin);
            break;
        case dailyThomNighttime:
                res = thomNightTime(meteoPoint.obsDataD[index].tMin, meteoPoint.obsDataD[index].rhMax);
                break;
        case dailyBIC:
                res = computeDailyBIC(meteoPoint.obsDataD[index].prec, meteoPoint.obsDataD[index].et0_hs);
                break;
        case dailyAirTemperatureRange:
                res = dailyThermalRange(meteoPoint.obsDataD[index].tMin, meteoPoint.obsDataD[index].tMax);
                break;
        case dailyAirTemperatureAvg:
                {
                    quality::qualityType qualityTavg = qualityCheck.syntacticQualitySingleValue(dailyAirTemperatureAvg, meteoPoint.obsDataD[index].tAvg);
                    if (qualityTavg == quality::accepted)
                    {
                        res = meteoPoint.obsDataD[index].tAvg;
                    }
                    else
                    {
                        res = dailyAverageT(meteoPoint.obsDataD[index].tMin, meteoPoint.obsDataD[index].tMax);
                        meteoPoint.obsDataD[index].tAvg = res;
                    }
                    break;
                }
        case dailyReferenceEvapotranspirationHS:
        {
            quality::qualityType qualityEtp = qualityCheck.syntacticQualitySingleValue(dailyReferenceEvapotranspirationHS, meteoPoint.obsDataD[index].et0_hs);
            if (qualityEtp == quality::accepted)
            {
                res = meteoPoint.obsDataD[index].et0_hs;
            }
            else
            {
                res = dailyEtpHargreaves(meteoPoint.obsDataD[index].tMin, meteoPoint.obsDataD[index].tMax, date, meteoPoint.latitude);
                meteoPoint.obsDataD[index].et0_hs = res;
            }
            break;
        }
        case dailyAirDewTemperatureAvg:
                res = dewPoint(meteoPoint.obsDataD[index].rhAvg, meteoPoint.obsDataD[index].tAvg); // RHavg, Tavg
                break;
        case dailyAirDewTemperatureMin:
                res = dewPoint(meteoPoint.obsDataD[index].rhMax, meteoPoint.obsDataD[index].tMin); // RHmax, Tmin
                break;
        case dailyAirDewTemperatureMax:
                res = dewPoint(meteoPoint.obsDataD[index].rhMin, meteoPoint.obsDataD[index].tMax); // RHmin, Tmax
                break;
        default:
                res = NODATA;
                break;
        }

        if (res != NODATA)
        {
            nrValidValues += 1;
        }

        outputValues.push_back(res);
        date = date.addDays(1);
    }

    *percValue = nrValidValues / meteoPoint.nrObsDataDaysD;
    if (nrValidValues > 0)
        return true;
    else
        return false;

}


bool elaborateDailyAggregatedVarFromHourly(meteoVariable myVar, Crit3DMeteoPoint meteoPoint, std::vector<float> &outputValues, Crit3DMeteoSettings *meteoSettings)
{

    float res;
    int nrValidValues = 0;
    Crit3DDate date = meteoPoint.obsDataH[0].date;

    for (int index = 0; index < meteoPoint.nrObsDataDaysH; index++)
    {

        switch(myVar)
        {
            case dailyThomHoursAbove:
                res = thomDailyNHoursAbove(meteoPoint.obsDataH[index].tAir, meteoPoint.obsDataH[index].rhAir, meteoSettings->getThomThreshold(), meteoSettings->getMinimumPercentage());
                break;
            case dailyThomMax:
                res = thomDailyMax(meteoPoint.obsDataH[index].tAir, meteoPoint.obsDataH[index].rhAir, meteoSettings->getMinimumPercentage());
                break;
            case dailyThomAvg:
                res = thomDailyMean(meteoPoint.obsDataH[index].tAir, meteoPoint.obsDataH[index].rhAir, meteoSettings->getMinimumPercentage());
                break;
            case dailyLeafWetness:
                res = dailyLeafWetnessComputation(meteoPoint.obsDataH[index].leafW, meteoSettings->getMinimumPercentage());
                break;
            default:
                res = NODATA;
                break;
        }

        if (res != NODATA)
        {
            nrValidValues += 1;
        }

        outputValues.push_back(res);
        date = date.addDays(1);
    }
    if (nrValidValues > 0)
        return true;
    else
        return false;

}


bool preElaboration(std::string *myError, Crit3DMeteoPointsDbHandler* meteoPointsDbHandler, Crit3DMeteoGridDbHandler* meteoGridDbHandler, Crit3DMeteoPoint* meteoPoint, bool isMeteoGrid, meteoVariable variable, meteoComputation elab1,
    QDate startDate, QDate endDate, std::vector<float> &outputValues, float* percValue, Crit3DMeteoSettings* meteoSettings, Crit3DElaborationSettings* elabSettings)
{

    bool preElaboration = false;
    bool automaticETP = elabSettings->getAutomaticETP();
    bool automaticTmed = elabSettings->getAutomaticTmed();

    switch(variable)
    {

        case dailyLeafWetness:
        {
            if ( loadHourlyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, leafWetness, QDateTime(startDate,QTime(0,0,0)), QDateTime(endDate,QTime(23,0,0))) > 0)
            {
                preElaboration = elaborateDailyAggregatedVar(dailyLeafWetness, *meteoPoint, outputValues, percValue, meteoSettings);
            }
            break;
        }

        case dailyThomDaytime:
        {
            if ( loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirRelHumidityMin, startDate, endDate) > 0)
            {
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMax, startDate, endDate) > 0)
                {
                    preElaboration = elaborateDailyAggregatedVar(dailyThomDaytime, *meteoPoint, outputValues, percValue, meteoSettings);
                }
            }
            break;
        }

        case dailyThomNighttime:
        {
            if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirRelHumidityMax, startDate, endDate) > 0)
            {
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMin, startDate, endDate) > 0)
                {
                    preElaboration = elaborateDailyAggregatedVar(dailyThomNighttime, *meteoPoint, outputValues, percValue, meteoSettings);
                }
            }
            break;
        }
        case dailyThomAvg: case dailyThomMax: case dailyThomHoursAbove:
        {

            if (loadHourlyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, airTemperature, QDateTime(startDate,QTime(0,0,0)), QDateTime(endDate,QTime(23,0,0)))  > 0)
            {
                if (loadHourlyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, airRelHumidity, QDateTime(startDate,QTime(0,0,0)), QDateTime(endDate,QTime(23,0,0)))  > 0)
                {
                    preElaboration = elaborateDailyAggregatedVar(variable, *meteoPoint, outputValues, percValue, meteoSettings);
                }
            }
            break;
        }
        case dailyBIC:
        {
            if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyReferenceEvapotranspirationHS, startDate, endDate) > 0)
            {
                preElaboration = true;
            }
            else if (automaticETP)
            {
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMin, startDate, endDate) > 0)
                {
                    if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMax, startDate, endDate) > 0)
                    {
                        preElaboration = elaborateDailyAggregatedVar(dailyReferenceEvapotranspirationHS, *meteoPoint, outputValues, percValue, meteoSettings);
                    }
                }
            }

            if (preElaboration)
            {
                preElaboration = false;
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyPrecipitation, startDate, endDate) > 0)
                {
                    preElaboration = elaborateDailyAggregatedVar(dailyBIC, *meteoPoint, outputValues, percValue, meteoSettings);
                }
            }
            break;
        }

        case dailyAirTemperatureRange:
        {
            if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMin, startDate, endDate) > 0)
            {
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMax, startDate, endDate) > 0)
                {
                    preElaboration = elaborateDailyAggregatedVar(dailyAirTemperatureRange, *meteoPoint, outputValues, percValue, meteoSettings);
                }
            }
            break;
        }
        case dailyAirDewTemperatureMax:
        {
            if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMax, startDate, endDate) > 0)
            {
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirRelHumidityMin, startDate, endDate) > 0)
                {
                    preElaboration = elaborateDailyAggregatedVar(dailyAirDewTemperatureMax, *meteoPoint, outputValues, percValue, meteoSettings);
                }
            }
            break;
        }

        case dailyAirDewTemperatureMin:
        {
            if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMin, startDate, endDate) > 0)
            {
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirRelHumidityMax, startDate, endDate) > 0)
                {
                    preElaboration = elaborateDailyAggregatedVar(dailyAirDewTemperatureMin, *meteoPoint, outputValues, percValue, meteoSettings);
                }
            }
            break;
        }

        case dailyAirTemperatureAvg:
        {
            if (loadDailyVarSeries_SaveOutput(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureAvg, startDate, endDate, outputValues) > 0)
            {
                preElaboration = true;
            }
            else if (automaticTmed)
            {
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMin, startDate, endDate) > 0 )
                {
                    if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMax, startDate, endDate) > 0)
                    {
                        preElaboration = elaborateDailyAggregatedVar(dailyAirTemperatureAvg, *meteoPoint, outputValues, percValue, meteoSettings);
                    }
                }
            }
            break;
        }

        case dailyReferenceEvapotranspirationHS:
        {
            if (loadDailyVarSeries_SaveOutput(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyReferenceEvapotranspirationHS, startDate, endDate, outputValues) > 0)
            {
                preElaboration = true;
            }
            else if (automaticETP)
            {
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMin, startDate, endDate) > 0)
                {
                    if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMax, startDate, endDate) > 0)
                    {
                        preElaboration = elaborateDailyAggregatedVar(dailyReferenceEvapotranspirationHS, *meteoPoint, outputValues, percValue, meteoSettings);
                    }
                }
            }
            break;
        }

        default:
        {
            switch(elab1)
            {
                case huglin:
                {
                    if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMin, startDate, endDate) > 0 )
                    {
                        if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMax, startDate, endDate) > 0 )
                        {
                            if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureAvg, startDate, endDate) > 0 )
                            {
                                preElaboration = true;
                            }
                            else if (automaticTmed)
                            {
                                preElaboration = elaborateDailyAggregatedVar(dailyAirTemperatureAvg, *meteoPoint, outputValues, percValue, meteoSettings);
                            }
                        }
                    }
                    break;
                }

            case winkler: case correctedDegreeDaysSum: case fregoni:
            {
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMin, startDate, endDate) > 0 )
                {
                    if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMax, startDate, endDate) > 0 )
                    {
                        preElaboration = true;
                    }
                }
                break;
            }

            case phenology:
            {
                if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMin, startDate, endDate) > 0 )
                {
                    if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyAirTemperatureMax, startDate, endDate) > 0 )
                    {
                        if (loadDailyVarSeries(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, dailyPrecipitation, startDate, endDate) > 0 )
                        {
                            preElaboration = true;
                        }
                    }
                }
                break;
             }

            default:
            {
                *percValue = loadDailyVarSeries_SaveOutput(myError, meteoPointsDbHandler, meteoGridDbHandler, meteoPoint, isMeteoGrid, variable, startDate, endDate, outputValues);

                preElaboration = ((*percValue) > 0);
                break;
            }

            }
            break;
        }
    }

    return preElaboration;
}


void extractValidValuesCC(std::vector<float> &outputValues)
{

    for (unsigned int i = 0;  i < outputValues.size(); i++)
    {
        if (outputValues.at(i) == NODATA)
        {
            outputValues.erase(outputValues.begin()+i);
        }
    }

}


void extractValidValuesWithThreshold(std::vector<float> &outputValues, float myThreshold)
{

    for (unsigned int i = 0;  i < outputValues.size(); i++)
    {
        if (outputValues.at(i) == NODATA || outputValues.at(i) < myThreshold)
        {
            outputValues.erase(outputValues.begin()+i);
        }
    }

}


//nYears   = 0         same year
//nYears   = 1,2,3...   betweend years 1,2,3...
float computeStatistic(std::vector<float> &inputValues, Crit3DMeteoPoint* meteoPoint, Crit3DClimate *clima, Crit3DDate firstDate, Crit3DDate lastDate, int nYears, meteoComputation elab1, meteoComputation elab2, Crit3DMeteoSettings* meteoSettings)
{

    std::vector<float> values;
    std::vector<float> valuesSecondElab;
    Crit3DDate presentDate;
    int numberOfDays;
    int nValidValues = 0;
    int nValues = 0;
    unsigned int index;

    float primary = NODATA;

    int firstYear = clima->yearStart();
    int lastYear = clima->yearEnd();
    float param1 = clima->param1();
    float param2 = clima->param2();

    // no secondary elab
    if (elab2 == noMeteoComp)
    {
        switch(elab1)
        {
            case lastDayBelowThreshold:
            {
                return computeLastDayBelowThreshold(inputValues, meteoPoint->obsDataD[0].date ,firstDate, lastDate, param1);
            }
            case winkler:
            {
                return computeWinkler(meteoPoint, firstDate, lastDate, meteoSettings->getMinimumPercentage());
            }
            case huglin:
            {
                return computeHuglin(meteoPoint, firstDate, lastDate, meteoSettings->getMinimumPercentage());
            }
            case fregoni:
            {
                return computeFregoni(meteoPoint, firstDate, lastDate, meteoSettings->getMinimumPercentage());
            }
            case correctedDegreeDaysSum:
            {
                return computeCorrectedSum(meteoPoint, firstDate, lastDate, param1, meteoSettings->getMinimumPercentage());
            }
            default:
            {

                int dOy;
                if (clima->getCurrentPeriodType() == dailyPeriod)
                {
                    dOy = getDoyFromDate(firstDate);
                }

                for (int presentYear = firstYear; presentYear <= lastYear; presentYear++)
                {

                    if ( (clima->getCurrentPeriodType() == dailyPeriod) )
                    {
                        firstDate = getDateFromDoy(presentYear, dOy);
                    }
                    firstDate.year = presentYear;
                    lastDate.year = presentYear;

                    if (nYears != NODATA)
                    {
                        if (nYears < 0)
                        {
                            firstDate.year = (presentYear + nYears);
                        }
                        else if (nYears > 0)
                        {
                            lastDate.year = (presentYear + nYears);
                        }
                    }

                    if ( (clima->getCurrentPeriodType() == dailyPeriod) )
                    {
                        numberOfDays = 1;
                    }
                    else
                    {
                        numberOfDays = difference(firstDate, lastDate) +1;
                    }

                    presentDate = firstDate;


                    for (int i = 0; i < numberOfDays; i++)
                    {

                        float value = NODATA;

                        if (meteoPoint->obsDataD[0].date > presentDate)
                        {
                            value = NODATA;
                        }
                        else
                        {
                            index = difference(meteoPoint->obsDataD[0].date, presentDate);
                            if (index < inputValues.size())
                            {
                                value = inputValues.at(index);
                            }
                        }

                        if (int(value) != NODATA)
                        {
                            values.push_back(value);
                            nValidValues = nValidValues + 1;
                        }

                        nValues = nValues + 1;

                        presentDate = presentDate.addDays(1);

                    }
                }

                if (nValidValues == 0)return NODATA;


                if (float(nValidValues) / float(nValues) * 100.f < meteoSettings->getMinimumPercentage()) return NODATA;

                return elaborations::statisticalElab(elab1, param1, values, nValidValues, meteoSettings->getRainfallThreshold());

                break;
            }
        }
    }
    // secondary elab
    else
    {
        int nTotYears = 0;
        int nValidYears = 0;
        valuesSecondElab.clear();

        int dOy;
        if (clima->getCurrentPeriodType() == dailyPeriod)
        {
            dOy = getDoyFromDate(firstDate);
        }

        for (int presentYear = firstYear; presentYear <= lastYear; presentYear++)
        {

            if ( (clima->getCurrentPeriodType() == dailyPeriod) )
            {
                firstDate = getDateFromDoy(presentYear, dOy);
            }

            firstDate.year = presentYear;
            lastDate.year = presentYear;

            if (nYears < 0)
            {
                firstDate.year = (presentYear + nYears);
            }
            else if (nYears > 0)
            {
                lastDate.year = (presentYear + nYears);
            }
            primary = NODATA;

            nValues = 0;
            nValidValues = 0;
            values.clear();

            switch(elab1)
            {
                case lastDayBelowThreshold:
                {
                    primary = computeLastDayBelowThreshold(inputValues, meteoPoint->obsDataD[0].date ,firstDate, lastDate, param1);
                    break;
                }
                case winkler:
                {
                    primary = computeWinkler(meteoPoint, firstDate, lastDate, meteoSettings->getMinimumPercentage());
                    break;
                }
                case huglin:
                {
                    primary = computeHuglin(meteoPoint, firstDate, lastDate, meteoSettings->getMinimumPercentage());
                    break;
                }
                case fregoni:
                {
                    primary = computeFregoni(meteoPoint, firstDate, lastDate, meteoSettings->getMinimumPercentage());
                    break;
                }
                case correctedDegreeDaysSum:
                {
                    primary = computeCorrectedSum(meteoPoint, firstDate, lastDate, param1, meteoSettings->getMinimumPercentage());
                    break;
                }
                default:
                {

                    if ( (clima->getCurrentPeriodType() == dailyPeriod) )
                    {
                        numberOfDays = 1;
                    }
                    else
                    {
                        numberOfDays = difference(firstDate, lastDate) +1;
                    }
                    presentDate = firstDate;
                    for (int i = 0; i < numberOfDays; i++)
                    {
                        float value = NODATA;

                        if (meteoPoint->obsDataD[0].date > presentDate)
                        {
                            value = NODATA;
                        }
                        else
                        {
                            index = difference(meteoPoint->obsDataD[0].date, presentDate);
                            if (index < inputValues.size())
                            {
                                value = inputValues.at(index);
                            }
                        }

                        if (int(value) != NODATA)
                        {
                            values.push_back(value);
                            nValidValues = nValidValues + 1;
                        }


                        nValues = nValues + 1;
                        presentDate = presentDate.addDays(1);

                    }

                    if (nValidValues > 0)
                    {
                        if (float(nValidValues) / float(nValues) * 100.f >= meteoSettings->getMinimumPercentage())
                        {
                            primary = elaborations::statisticalElab(elab1, param1, values, nValidValues, meteoSettings->getRainfallThreshold());
                        }
                    }

                    break;

                }
            }

            if (primary != NODATA)
            {
                valuesSecondElab.push_back(primary);
                nValidYears = nValidYears + 1;
            }

            nTotYears = nTotYears + 1;

        } // end for

        if (nTotYears == 0)
        {
            return NODATA;
        }
        else if (float(nValidYears) / float(nTotYears) * 100.f < meteoSettings->getMinimumPercentage())
        {
            return NODATA;
        }
        else
        {
            switch(elab2)
            {
                case trend:
                    return elaborations::statisticalElab(elab2, firstYear, valuesSecondElab, nValidYears, meteoSettings->getRainfallThreshold());
                default:
                    return elaborations::statisticalElab(elab2, param2, valuesSecondElab, nValidYears, meteoSettings->getRainfallThreshold());
            }
        }
    }
}

QString getTable(QString elab)
{
    QStringList words = elab.split('_');
    QString periodTypeStr = words[2];
    QString tableName = "climate_"+periodTypeStr.toLower();

    return tableName;
}

int getClimateIndexFromElab(QDate myDate, QString elab)
{

    QStringList words = elab.split('_');
    QString periodTypeStr = words[2];

    period periodType = getPeriodTypeFromString(periodTypeStr);

    switch(periodType)
    {
    case annualPeriod: case genericPeriod:
            return 1;
    case decadalPeriod:
            return decadeFromDate(myDate);
    case monthlyPeriod:
            return myDate.month();
    case seasonalPeriod:
            return getSeasonFromDate(myDate);
    case dailyPeriod:
            return myDate.dayOfYear();
    default:
            return NODATA;
    }
}

period getPeriodTypeFromString(QString periodStr)
{

    if (periodStr == "Daily")
        return dailyPeriod;
    if (periodStr == "Decadal")
        return decadalPeriod;
    if (periodStr == "Monthly")
        return monthlyPeriod;
    if (periodStr == "Seasonal")
        return seasonalPeriod;
    if (periodStr == "Annual")
        return annualPeriod;
    if (periodStr == "Generic")
        return genericPeriod;

    return noPeriodType;

}

int nParameters(meteoComputation elab)
{
    switch(elab)
    {
    case average:
        return 0;
    case maxInList:
        return 0;
    case minInList:
        return 0;
    case sum:
        return 0;
    case avgAbove:
        return 1;
    case stdDevAbove:
        return 1;
    case sumAbove:
        return 1;
    case daysAbove:
        return 1;
    case daysBelow:
        return 1;
    case consecutiveDaysAbove:
        return 1;
    case consecutiveDaysBelow:
        return 1;
    case percentile:
        return 1;
    case prevailingWindDir:
        return 0;
    case correctedDegreeDaysSum:
        return 1;
    case trend:
        return 0;
    case mannKendall:
        return 0;
    case differenceWithThreshold:
        return 1;
    case lastDayBelowThreshold:
        return 1;
    default:
        return 0;
    }

}

/*
 * // LC old
int getClimateIndexFromDate(QDate myDate, period periodType)
{
    switch(periodType)
    {
    case annualPeriod: case genericPeriod:
            return 1;
    case decadalPeriod:
            return decadeFromDate(myDate);
    case monthlyPeriod:
            return myDate.month();
    case seasonalPeriod:
            return getSeasonFromDate(myDate);
    case dailyPeriod:
            return myDate.dayOfYear();
    default:
            return NODATA;
    }
}
*/
