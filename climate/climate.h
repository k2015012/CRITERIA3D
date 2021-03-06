#ifndef CLIMATE_H
#define CLIMATE_H

    class QString;
    class QDate;

    #ifndef METEO_H
        #include "meteo.h"
    #endif

    #ifndef STATISTICS_H
        #include "statistics.h"
    #endif

    #ifndef DBMETEOPOINTS_H
        #include "dbMeteoPoints.h"
    #endif

    #ifndef DBMETEOGRID_H
        #include "dbMeteoGrid.h"
    #endif

    #ifndef CRIT3DCLIMATE_H
        #include "crit3dClimate.h"
    #endif


    const std::map<std::string, int> MapElabWithParam = {
      { "differenceWithThreshold", 1 },
      { "lastDayBelowThreshold", 1 },
      { "sumAbove", 1 },
      { "avgAbove", 1 },
      { "stdDevAbove", 1 },
      { "percentile", 1 },
      { "daysAbove", 1 },
      { "daysBelow", 1 },
      { "consecutiveDaysAbove", 1 },
      { "consecutiveDaysBelow", 1 },
      { "correctedDegreeDaysSum", 1 }
    };

    bool elaborationOnPoint(std::string *myError, Crit3DMeteoPointsDbHandler* meteoPointsDbHandler, Crit3DMeteoGridDbHandler* meteoGridDbHandler,
        Crit3DMeteoPoint* meteoPointTemp, Crit3DClimate* clima, bool isMeteoGrid, QDate startDate, QDate endDate, bool isAnomaly, Crit3DMeteoSettings *meteoSettings);

    frequencyType getAggregationFrequency(meteoVariable myVar);

    bool elaborateDailyAggregatedVar(meteoVariable myVar, Crit3DMeteoPoint meteoPoint, std::vector<float> &outputValues, float* percValue, Crit3DMeteoSettings *meteoSettings);

    bool elaborateDailyAggregatedVarFromDaily(meteoVariable myVar, Crit3DMeteoPoint meteoPoint, std::vector<float> &outputValues, float* percValue);

    bool elaborateDailyAggregatedVarFromHourly(meteoVariable myVar, Crit3DMeteoPoint meteoPoint, std::vector<float> &outputValues, Crit3DMeteoSettings *meteoSettings);

    bool anomalyOnPoint(Crit3DMeteoPoint* meteoPoint, float refValue);

    bool passingClimateToAnomaly(std::string *myError, Crit3DMeteoPoint* meteoPointTemp, Crit3DClimate* clima, Crit3DMeteoPoint *meteoPoints, int nrMeteoPoints, Crit3DElaborationSettings *elabSettings);

    bool climateOnPoint(std::string *myError, Crit3DMeteoPointsDbHandler* meteoPointsDbHandler, Crit3DMeteoGridDbHandler* meteoGridDbHandler, Crit3DClimate* clima, Crit3DMeteoPoint *meteoPointTemp, std::vector<float> &outputValues, bool isMeteoGrid, QDate startDate, QDate endDate, bool changeDataSet, Crit3DMeteoSettings *meteoSettings);

    bool climateTemporalCycle(std::string *myError, Crit3DClimate* clima, std::vector<float> &outputValues, Crit3DMeteoPoint* meteoPoint, meteoComputation elab1, meteoComputation elab2, Crit3DMeteoSettings *meteoSettings);

    float thomDayTime(float tempMax, float relHumMinAir);

    float thomNightTime(float tempMin, float relHumMaxAir);

    float thomH(float tempAvg, float relHumAvgAir);

    int thomDailyNHoursAbove(float *tempAvg, float *relHumAvgAir, float thomthreshold, float minimumPercentage);

    float thomDailyMax(float *tempAvg, float* relHumAvgAir, float minimumPercentage);

    float thomDailyMean(float *tempAvg, float* relHumAvgAir, float minimumPercentage);

    float dailyLeafWetnessComputation(int *leafW, float minimumPercentage);

    float computeDailyBIC(float prec, float etp);

    float dailyThermalRange(float Tmin, float Tmax);

    float dailyAverageT(float Tmin, float Tmax);

    float dailyEtpHargreaves(float Tmin, float Tmax, Crit3DDate date, double latitude);

    float dewPoint(float relHumAir, float tempAir);

    float computeLastDayBelowThreshold(std::vector<float> &inputValues, Crit3DDate firstDateDailyVar, Crit3DDate firstDate, Crit3DDate finishDate, float param1);

    float computeWinkler(Crit3DMeteoPoint* meteoPoint, Crit3DDate firstDate, Crit3DDate finishDate, float minimumPercentage);

    float computeHuglin(Crit3DMeteoPoint* meteoPoint, Crit3DDate firstDate, Crit3DDate finishDate, float minimumPercentage);

    float computeFregoni(Crit3DMeteoPoint* meteoPoint, Crit3DDate firstDate, Crit3DDate finishDate, float minimumPercentage);

    float computeCorrectedSum(Crit3DMeteoPoint* meteoPoint, Crit3DDate firstDate, Crit3DDate finishDate, float param, float minimumPercentage);

    bool preElaboration(std::string *myError, Crit3DMeteoPointsDbHandler* meteoPointsDbHandler, Crit3DMeteoGridDbHandler* meteoGridDbHandler, Crit3DMeteoPoint* meteoPoint, bool isMeteoGrid, meteoVariable variable, meteoComputation elab1,
        QDate startDate, QDate endDate, std::vector<float> &outputValues, float* percValue, Crit3DMeteoSettings *meteoSettings, Crit3DElaborationSettings *elabSettings);

    float loadDailyVarSeries(std::string *myError, Crit3DMeteoPointsDbHandler *meteoPointsDbHandler,
                             Crit3DMeteoGridDbHandler *meteoGridDbHandler, Crit3DMeteoPoint* meteoPoint,
                             bool isMeteoGrid, meteoVariable variable, QDate first, QDate last);

    float loadDailyVarSeries_SaveOutput(std::string *myError, Crit3DMeteoPointsDbHandler *meteoPointsDbHandler,
            Crit3DMeteoGridDbHandler *meteoGridDbHandler, Crit3DMeteoPoint* meteoPoint, bool isMeteoGrid,
            meteoVariable variable, QDate first, QDate last, std::vector<float> &outputValues);

    float loadHourlyVarSeries(std::string *myError, Crit3DMeteoPointsDbHandler *meteoPointsDbHandler, Crit3DMeteoGridDbHandler *meteoGridDbHandler, Crit3DMeteoPoint* meteoPoint, bool isMeteoGrid, meteoVariable variable, QDateTime first, QDateTime last);

    void extractValidValuesCC(std::vector<float> &outputValues);

    void extractValidValuesWithThreshold(std::vector<float> &outputValues, float myThreshold);

    float computeStatistic(std::vector<float> &inputValues, Crit3DMeteoPoint* meteoPoint, Crit3DClimate* clima, Crit3DDate firstDate, Crit3DDate lastDate, int nYears, meteoComputation elab1, meteoComputation elab2, Crit3DMeteoSettings *meteoSettings);

    QString getTable(QString elab);

    int getClimateIndexFromElab(QDate myDate, QString elab);

    period getPeriodTypeFromString(QString periodStr);

    int nParameters(meteoComputation elab);

    //int getClimateIndexFromDate(QDate myDate, period periodType);

#endif // CLIMATE_H
