#include <stdio.h>
#include <cmath>

#include "commonConstants.h"
#include "spatialControl.h"
#include "interpolation.h"
#include "statistics.h"


bool checkData(Crit3DQuality* myQuality, meteoVariable myVar, frequencyType myFrequency, Crit3DMeteoPoint* meteoPoints, int nrMeteoPoints,
                              Crit3DTime myTime, Crit3DInterpolationSettings* spatialQualityInterpolationSettings, bool checkSpatial)
{
    if (nrMeteoPoints == 0)
        return false;

    // assign data
    for (int i = 0; i < nrMeteoPoints; i++)
        meteoPoints[i].currentValue = meteoPoints[i].getMeteoPointValue(myTime, myVar, myFrequency);

    // quality control - synthctic
    myQuality->syntacticQualityControl(myVar, meteoPoints, nrMeteoPoints);

    // quality control - spatial
    if (checkSpatial && myVar != precipitation && myVar != dailyPrecipitation
                     && myVar != globalIrradiance && myVar != dailyGlobalRadiation)
    {
        spatialQualityControl(myVar, meteoPoints, nrMeteoPoints, spatialQualityInterpolationSettings);
    }

    return true;
}

// check quality and pass good data to interpolation
bool checkAndPassDataToInterpolation(Crit3DQuality* myQuality, meteoVariable myVar, frequencyType myFrequency,
                                      Crit3DMeteoPoint* meteoPoints, int nrMeteoPoints,
                                      Crit3DTime myTime, Crit3DInterpolationSettings* SQinterpolationSettings,
                                      Crit3DInterpolationSettings* interpolationSettings,
                                      std::vector <Crit3DInterpolationDataPoint> &myInterpolationPoints,
                                      bool checkSpatial)
{
    if (! checkData(myQuality, myVar, myFrequency, meteoPoints, nrMeteoPoints, myTime, SQinterpolationSettings, checkSpatial)) return false;

    // return true if at least one valid data
    return passDataToInterpolation(meteoPoints, nrMeteoPoints, myInterpolationPoints, interpolationSettings);
}


float findThreshold(meteoVariable myVar, float value, float stdDev, float nrStdDev, float stdDevZ, float minDistance)
{
    float zWeight, distWeight, threshold;

    if (   myVar == precipitation
        || myVar == dailyPrecipitation)
    {
        distWeight = maxValue(1.f, minDistance / 2000.f);
        if (value < PREC_THRESHOLD)
            threshold = maxValue(5.f, distWeight + stdDev * (nrStdDev + 1));
        else
            return 800.f;
    }
    else if (   myVar == airTemperature
             || myVar == airDewTemperature
             || myVar == dailyAirTemperatureMax
             || myVar == dailyAirTemperatureMin
             || myVar == dailyAirTemperatureAvg )
    {
        threshold = 1.f;
        zWeight = stdDevZ / 100.f;
        distWeight = minDistance / 5000.f;

        threshold = minValue(minValue(distWeight + threshold + zWeight, 12.f) + stdDev * nrStdDev, 15.f);
    }
    else if (   myVar == airRelHumidity
             || myVar == dailyAirRelHumidityMax
             || myVar == dailyAirRelHumidityMin
             || myVar == dailyAirRelHumidityAvg )
    {
        threshold = 8.f;
        zWeight = stdDevZ / 100.f;
        distWeight = minDistance / 1000.f;
        threshold += zWeight + distWeight + stdDev * nrStdDev;
    }
    else if (   myVar == windIntensity
             || myVar == dailyWindIntensityAvg
             || myVar == dailyWindIntensityMax)
    {
        threshold = 1.f;
        zWeight = stdDevZ / 50.f;
        distWeight = minDistance / 2000.f;
        threshold += zWeight + distWeight + stdDev * nrStdDev;
    }
    else if (   myVar == globalIrradiance
             || myVar == dailyGlobalRadiation )
        threshold = maxValue(stdDev * (nrStdDev + 1.f), 5.f);

    else if (myVar == atmTransmissivity)
        threshold = maxValue(stdDev * nrStdDev, 0.5f);
    else
        threshold = stdDev * nrStdDev;

    return threshold;
}


bool computeResiduals(meteoVariable myVar, Crit3DMeteoPoint* meteoPoints, int nrMeteoPoints,
                      std::vector <Crit3DInterpolationDataPoint> &interpolationPoints, Crit3DInterpolationSettings* settings)
{

    if (myVar == noMeteoVar) return false;

    float myValue, interpolatedValue;
    interpolatedValue = NODATA;
    myValue = NODATA;
    std::vector <float> myProxyValues;

    for (int i = 0; i < nrMeteoPoints; i++)
    {
        meteoPoints[i].residual = NODATA;

        if (meteoPoints[i].quality == quality::accepted)
        {
            myValue = meteoPoints[i].currentValue;

            interpolatedValue = interpolate(interpolationPoints, settings, myVar, float(meteoPoints[i].point.utm.x),
                                            float(meteoPoints[i].point.utm.y),
                                            float(meteoPoints[i].point.z),
                                            myProxyValues,
                                            false);

            if (  myVar == precipitation
               || myVar == dailyPrecipitation)
            {
                if (myValue != NODATA)
                    if (myValue < PREC_THRESHOLD) myValue=0.;

                if (interpolatedValue != NODATA)
                    if (interpolatedValue < PREC_THRESHOLD) interpolatedValue=0.;
            }

            // TODO derived var

            if ((interpolatedValue != NODATA) && (myValue != NODATA))
                meteoPoints[i].residual = interpolatedValue - myValue;
        }
    }

    return true;
}

float computeErrorCrossValidation(meteoVariable myVar, std::vector <Crit3DMeteoPoint> myPoints, const Crit3DTime& myTime)
{
    return 0;
    std::vector <float> obsValues, estValues;
    float myValue, myEstimate, myResidual;

    frequencyType myFreq = getFrequency(myVar);

    for (int i=0; i<myPoints.size(); i++)
    {
        if (myPoints.at(i).active)
        {
            myValue = myPoints.at(i).getMeteoPointValue(myTime, myVar, myFreq);
            myResidual = myPoints.at(i).residual;

            if (myValue != NODATA && myResidual != NODATA)
            {
                myEstimate = myValue + myResidual;
                obsValues.push_back(myValue);
                estValues.push_back(myEstimate);
            }
        }
    }

    if (obsValues.size() > 0)
        return statistics::meanError(obsValues, estValues);
}

void topographicDistanceOptimize(meteoVariable myVar,
                                 std::vector <Crit3DMeteoPoint> &myPoints,
                                 std::vector <Crit3DInterpolationDataPoint> interpolationPoints,
                                 Crit3DInterpolationSettings* mySettings,
                                 const Crit3DTime &myTime)
{
    float avgError, bestError, bestK;
    float kh, kz;

    kh = kz = 0;

    bestError = NODATA;

    return;

//    do while (kz <= 256)
//    {
//        computeResiduals(myVar, myPoints, nrMeteoPoints, interpolationPoints, mySettings);

//        avgError = computeErrorCrossValidation(myVar, myPoints, const Crit3DTime &myTime)
//        if (bestError = NODATA || avgError < bestError)
//        {
//            bestError = avgError;
//            bestK = kz;
//        }
//        kz = (kz == 0 ? 1 : kz*2);
//    }

//    kz = bestK;

//    khTAD = 0
//    bestMAE = Definitions.NO_DATA
//    While (khTAD <= 1000000)
//        InterpolationCmd.InterpolationCV myVar, False, True, True
//        avgError = InterpolationCmd.computeMAECrossValidation(myVar)
//        If bestMAE = Definitions.NO_DATA Or avgError < bestMAE Then
//            bestMAE = avgError
//            bestK = khTAD
//        End If
//        khTAD = IIf(khTAD = 0, 1, khTAD * 2#)
//    Wend

//    khTAD = bestK

}

void bestDetrending()
{

}

void spatialQualityControl(meteoVariable myVar, Crit3DMeteoPoint* meteoPoints, int nrMeteoPoints,
                           Crit3DInterpolationSettings *settings)
{
    int i;
    float stdDev, stdDevZ, minDist, myValue, myResidual;
    std::vector <int> listIndex;
    std::vector <float> listResiduals;
    std::vector <float> myProxyValues;
    std::vector <Crit3DInterpolationDataPoint> myInterpolationPoints;

    if (passDataToInterpolation(meteoPoints, nrMeteoPoints, myInterpolationPoints, settings))
    {
        // detrend
        if (! preInterpolation(myInterpolationPoints, settings, myVar))
            return;

        // compute residuals
        if (! computeResiduals(myVar, meteoPoints, nrMeteoPoints, myInterpolationPoints, settings))
            return;

        // re-load data
        passDataToInterpolation(meteoPoints, nrMeteoPoints, myInterpolationPoints, settings);

        for (i = 0; i < nrMeteoPoints; i++)
            if (meteoPoints[i].quality == quality::accepted)
            {
                if (neighbourhoodVariability(myInterpolationPoints, settings, float(meteoPoints[i].point.utm.x),
                         float(meteoPoints[i].point.utm.y),float(meteoPoints[i].point.z),
                         10, &stdDev, &stdDevZ, &minDist))
                {
                    myValue = meteoPoints[i].currentValue;
                    myResidual = meteoPoints[i].residual;
                    stdDev = maxValue(stdDev, myValue/100.f);
                    if (fabs(myResidual) > findThreshold(myVar, myValue, stdDev, 2, stdDevZ, minDist))
                    {
                        listIndex.push_back(i);
                        meteoPoints[i].quality = quality::wrong_spatial;
                    }
                }
            }

        if (listIndex.size() > 0)
        {
            if (passDataToInterpolation(meteoPoints, nrMeteoPoints, myInterpolationPoints, settings))
            {
                preInterpolation(myInterpolationPoints, settings, myVar);

                float interpolatedValue;
                for (i=0; i < int(listIndex.size()); i++)
                {
                    interpolatedValue = interpolate(myInterpolationPoints, settings, myVar,
                                            float(meteoPoints[listIndex[i]].point.utm.x),
                                            float(meteoPoints[listIndex[i]].point.utm.y),
                                            float(meteoPoints[listIndex[i]].point.z),
                                            myProxyValues, false);

                    myValue = meteoPoints[listIndex[i]].currentValue;

                    listResiduals.push_back(interpolatedValue - myValue);
                }

                passDataToInterpolation(meteoPoints, nrMeteoPoints, myInterpolationPoints, settings);

                for (i=0; i < int(listIndex.size()); i++)
                {
                    if (neighbourhoodVariability(myInterpolationPoints, settings, float(meteoPoints[listIndex[i]].point.utm.x),
                             float(meteoPoints[listIndex[i]].point.utm.y),
                             float(meteoPoints[listIndex[i]].point.z),
                             10, &stdDev, &stdDevZ, &minDist))
                    {
                        myResidual = listResiduals[i];

                        myValue = meteoPoints[listIndex[i]].currentValue;

                        if (fabs(myResidual) > findThreshold(myVar, myValue, stdDev, 3, stdDevZ, minDist))
                            meteoPoints[listIndex[i]].quality = quality::wrong_spatial;
                        else
                            meteoPoints[listIndex[i]].quality = quality::accepted;
                    }
                    else
                        meteoPoints[listIndex[i]].quality = quality::accepted;
                }
            }
        }
    }
}

bool passDataToInterpolation(Crit3DMeteoPoint* meteoPoints, int nrMeteoPoints,
                            std::vector <Crit3DInterpolationDataPoint> &myInterpolationPoints,
                            Crit3DInterpolationSettings* mySettings)
{
    int myCounter = 0;
    float xMin=NODATA, xMax, yMin, yMax;

    myInterpolationPoints.clear();

    for (int i = 0; i < nrMeteoPoints; i++)
    {
        if (meteoPoints[i].quality == quality::accepted)
        {
            Crit3DInterpolationDataPoint myPoint;

            myPoint.index = i;
            myPoint.value = meteoPoints[i].currentValue;
            myPoint.point->utm.x = float(meteoPoints[i].point.utm.x);
            myPoint.point->utm.y = float(meteoPoints[i].point.utm.y);
            myPoint.point->z = float(meteoPoints[i].point.z);
            myPoint.proxyValues = meteoPoints[i].proxyValues;
            myPoint.isActive = true;

            if (xMin = NODATA)
            {
                xMin = (float)myPoint.point->utm.x;
                xMax = (float)myPoint.point->utm.x;
                yMin = (float)myPoint.point->utm.y;
                yMax = (float)myPoint.point->utm.y;
            }
            else
            {
                xMin = minValue(xMin, (float)myPoint.point->utm.x);
                xMax = maxValue(xMax, (float)myPoint.point->utm.x);
                yMin = minValue(yMin, (float)myPoint.point->utm.y);
                yMax = maxValue(yMax, (float)myPoint.point->utm.y);
            }

            myInterpolationPoints.push_back(myPoint);
            myCounter++;
        }
    }

    if (myCounter > 0)
    {
        mySettings->computeShepardInitialRadius((xMax - xMin)*(yMax-yMin), myCounter);
        return true;
    }
    else
        return false;
}
