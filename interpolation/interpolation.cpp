/*!
    \copyright 2016 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by A.R.P.A. Emilia-Romagna

    CRITERIA3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CRITERIA3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA3D.  If not, see <http://www.gnu.org/licenses/>.

    contacts:
    fausto.tomei@gmail.com
    ftomei@arpae.it
*/


#include <stdlib.h>

#include <math.h>
#include <vector>

#include "commonConstants.h"
#include "basicMath.h"
#include "statistics.h"
#include "furtherMathFunctions.h"
#include "meteoPoint.h"
#include "gis.h"
#include "interpolation.h"


using namespace std;

vector <Crit3DInterpolationDataPoint> interpolationPointList;
vector <Crit3DInterpolationDataPoint> precBinaryPointList;


void clearInterpolationPoints()
{
    interpolationPointList.clear();
}

bool addInterpolationPoint(int myIndex, float myValue, float myX, float myY, float myZ, vector <float> myProxyValues)
{
    Crit3DInterpolationDataPoint myPoint;

    myPoint.index = myIndex;
    myPoint.value = myValue;
    myPoint.point->utm.x = myX;
    myPoint.point->utm.y = myY;
    myPoint.isActive = true;
    myPoint.point->z = myZ;
    myPoint.proxyValues = myProxyValues;

    interpolationPointList.push_back(myPoint);

    return (true);
}


void assignDistances(vector <Crit3DInterpolationDataPoint> *myPoints, float x, float y, float z)
{
    for (unsigned long i = 0; i < myPoints->size() ; i++)
    {
        myPoints->at(i).distance = gis::computeDistance(x, y, float((myPoints->at(i)).point->utm.x) , float((myPoints->at(i)).point->utm.y));
        myPoints->at(i).deltaZ = float(fabs(myPoints->at(i).point->z - z));
    }
}

float getMinHeight()
{
    float myZmin = NODATA;

    for (long i = 0; i < long(interpolationPointList.size()); i++)
        if (interpolationPointList.at(i).point->z != NODATA)
            if (myZmin == NODATA || interpolationPointList.at(i).point->z < myZmin)
                myZmin = float(interpolationPointList.at(i).point->z);
    return myZmin;
}

float getMaxHeight()
{
    float zMax;
    zMax = NODATA;

    for (long i = 1; i < long(interpolationPointList.size()); i++)
        if ((interpolationPointList.at(i)).value != NODATA && (interpolationPointList.at(i)).isActive)
            if (zMax == NODATA || (interpolationPointList.at(i)).point->z > zMax)
                zMax = float(interpolationPointList.at(i).point->z);


    return zMax;
}

int sortPointsByDistance(int maxIndex, vector <Crit3DInterpolationDataPoint> myPoints, vector <Crit3DInterpolationDataPoint>* myValidPoints)
{   
    int i, first, index;
    float min_value;
    int* indici_ordinati;
    int* indice_minimo;
    int outIndex;

    if (myPoints.size() == 0) return 0;

    indici_ordinati = (int *) calloc(maxIndex + 1, sizeof(int));
    indice_minimo = (int *) calloc(myPoints.size(), sizeof(int));

    first = 0;
    index = 0;

    bool exit = false;

    while (index <= maxIndex && !exit)
    {
        if (first == 0)
        {
            i = 0;
            while ((! myPoints.at(i).isActive || (myPoints.at(i).distance == 0)) && (i < int(myPoints.size())-1))
                i++;

            if (i == int(myPoints.size())-1 && ! myPoints.at(i).isActive)
                exit=true;
            else
            {
                first = 1;
                indice_minimo[first-1] = i;
                min_value = myPoints.at(i).distance;
            }
        }
        else
            min_value = myPoints.at(indice_minimo[first-1]).distance;

        if (!exit)
        {
            for (i = indice_minimo[first-1] + 1; i < int(myPoints.size()); i++)
                if (myPoints.at(i).distance < min_value)
                    if (myPoints.at(i).isActive)
                        if (myPoints.at(i).distance > 0)
                        {
                            first++;
                            min_value = myPoints.at(i).distance;
                            indice_minimo[first-1] = i;
                        }

            indici_ordinati[index] = indice_minimo[first-1];
            myPoints.at(indice_minimo[first-1]).isActive = false;
            index++;
            first--;
        }
    }

    outIndex = minValue(index - 1, maxIndex);
    (*myValidPoints).resize(outIndex+1);

    for (i=0; i<outIndex+1; i++)
    {
        myPoints.at(indici_ordinati[i]).isActive = true;
        (*myValidPoints).at(i) = myPoints.at(indici_ordinati[i]);
    }

    return outIndex+1;
}


bool neighbourhoodVariability(float x, float y, float z, int nMax,
                              float* devSt, float* devStDeltaZ, float* minDistance)
{
    int i, max_points;
    float* dataNeighborhood;
    float myValue;
    vector <float> deltaZ;
    vector <Crit3DInterpolationDataPoint> validPoints;

    assignDistances(&interpolationPointList, x, y, z);
    max_points = sortPointsByDistance(nMax, interpolationPointList, &validPoints);

    if (max_points > 1)
    {
        dataNeighborhood = (float *) calloc (max_points, sizeof(float));

        for (i=0; i<max_points; i++)
        {
            myValue = validPoints.at(i).value;
            dataNeighborhood[i] = myValue;
        }

        *devSt = statistics::standardDeviation(dataNeighborhood, max_points);

        *minDistance = validPoints.at(0).distance;

        deltaZ.clear();
        if (z != NODATA)
            deltaZ.push_back(1);

        for (i=0; i<max_points;i++)
            if ((validPoints.at(i)).point->z != NODATA)
                deltaZ.push_back(fabs(((float)(validPoints.at(i)).point->z) - z));

        *devStDeltaZ = statistics::mean(deltaZ.data(), int(deltaZ.size()));

        return true;
    }
    else
        return false;
}


void regressionSimple(int proxyPosition, bool isZeroIntercept, float* myCoeff, float* myIntercept, float* myR2)
{
    long i;
    float myProxyValue;
    Crit3DInterpolationDataPoint* myPoint;
    vector <float> myValues, myZ;

    *myCoeff = NODATA;
    *myIntercept = NODATA;
    *myR2 = NODATA;

    myValues.clear();
    myZ.clear();

    for (i = 0; i < long(interpolationPointList.size()); i++)
    {
        myPoint = &(interpolationPointList.at(i));
        myProxyValue = NODATA;
        if (myPoint->isActive)
        {
            myProxyValue = myPoint->getProxy(proxyPosition);
            if (myProxyValue != NODATA)
            {
                myValues.push_back(myPoint->value);
                myZ.push_back(myProxyValue);
            }
        }
    }

    if (myValues.size() >= MIN_REGRESSION_POINTS)
        statistics::linearRegression((float*)(myZ.data()), (float*)(myValues.data()), (long)(myZ.size()), isZeroIntercept,
                                     myIntercept, myCoeff, myR2);
}

bool regressionGeneric(int proxyPos, bool isZeroIntercept, Crit3DInterpolationSettings* mySettings)
{
    float q, m, r2;

    regressionSimple(proxyPos, isZeroIntercept, &m, &q, &r2);
    Crit3DProxyInterpolation* myProxy = mySettings->getProxy(proxyPos);

    myProxy->setRegressionSlope(m);
    myProxy->setRegressionR2(r2);

    return (r2 >= mySettings->getGenericPearsonThreshold());
}


bool regressionSimpleT(meteoVariable myVar, int orogProxyPos, Crit3DInterpolationSettings* mySettings)
{
    float q, m, r2;

    Crit3DProxyInterpolation* myProxyOrog = mySettings->getProxy(orogProxyPos);
    myProxyOrog->initializeOrography();

    regressionSimple(orogProxyPos, false, &m, &q, &r2);

    if (r2 < mySettings->getGenericPearsonThreshold())
        return false;

    myProxyOrog->setRegressionSlope(m);
    myProxyOrog->setRegressionR2(r2);

    // only pre-inversion data
    if (m > 0)
    {
        myProxyOrog->setInversionLapseRate(m);

        float maxZ = minValue(getMaxHeight(), mySettings->getMaxHeightInversion());
        myProxyOrog->setLapseRateH1(maxZ);
        myProxyOrog->setRegressionSlope(mySettings->getCurrentClimateLapseRate(myVar));
        myProxyOrog->setInversionIsSignificative(true);
    }

    return true;
}


float findHeightIntervalAvgValue(float heightInf, float heightSup, float maxPointsZ)
{
    long myIndex;
    float myValue, mySum, nValues;

    mySum = 0.;
    nValues = 0;

    for (myIndex = 0; myIndex < long(interpolationPointList.size()); myIndex++)
         if ((interpolationPointList.at(myIndex)).point->z != NODATA && (interpolationPointList.at(myIndex)).isActive)
            if ((interpolationPointList.at(myIndex)).point->z >= heightInf && (interpolationPointList.at(myIndex)).point->z <= heightSup)
            {
                myValue = (interpolationPointList.at(myIndex)).value;
                if (myValue != NODATA)
                {
                    mySum += myValue;
                    nValues ++;
                }
            }

    if (nValues > 1 || (nValues > 0 && heightSup >= maxPointsZ))
        return (mySum / nValues);
    else
        return NODATA;
}

bool regressionOrographyT(meteoVariable myVar, int orogProxyPos, bool climateExists, Crit3DInterpolationSettings* mySettings)
{
    long i;
    float heightInf, heightSup;
    float myAvg;
    vector <float> myData1, myData2;
    vector <float> myHeights1, myHeights2;
    vector <float> myIntervalsHeight, myIntervalsHeight1, myIntervalsHeight2;
    vector <float> myIntervalsValues, myIntervalsValues1, myIntervalsValues2;
    float m, q, r2;
    float r2_values, r2_intervals;
    float q1, m1, r21, q2, m2, r22;
    float mySignificativeR2, mySignificativeR2Inv;
    float maxPointsZ, deltaZ;
    float climateLapseRate, lapseRateT0, lapseRateT1, lapseRateH1_;
    float DELTAZ_INI = 80.;
    float maxHeightInv = mySettings->getMaxHeightInversion();

    Crit3DProxyInterpolation* myProxyOrog = mySettings->getProxy(orogProxyPos);

    mySignificativeR2 = maxValue(mySettings->getGenericPearsonThreshold(), float(0.2));
    mySignificativeR2Inv = maxValue(mySettings->getGenericPearsonThreshold(), float(0.1));

    /*! initialize */
    myProxyOrog->initializeOrography();

    if (climateExists)
        climateLapseRate = mySettings->getCurrentClimateLapseRate(myVar);
    else
        climateLapseRate = 0.;

    myProxyOrog->setRegressionSlope(climateLapseRate);

    maxPointsZ = getMaxHeight();

    /*! not enough data to define a curve (use climate) */
    if (interpolationPointList.size() < MIN_REGRESSION_POINTS)
        return true;

    /*! find intervals averages */
    heightInf = getMinHeight();
    heightSup = heightInf;
    deltaZ = DELTAZ_INI;
    while (heightSup <= maxPointsZ)
    {
        myAvg = NODATA;
        while (myAvg == NODATA)
        {
            heightSup = heightSup + deltaZ;
            if (heightSup > maxPointsZ && !(maxPointsZ == 0 && heightInf == 0))
                heightSup = maxPointsZ + (maxPointsZ - heightInf);

            myAvg = findHeightIntervalAvgValue(heightInf, heightSup, maxPointsZ);
        }
        myIntervalsHeight.push_back((heightSup + heightInf) / float(2.));
        myIntervalsValues.push_back(myAvg);

        deltaZ = DELTAZ_INI * exp(heightInf / maxHeightInv);
        heightInf = heightSup;
    }

    /*! find inversion height */
    lapseRateT1 = myIntervalsValues.at(0);
    myProxyOrog->setLapseRateH1(myIntervalsHeight.at(0));
    for (i = 1; i < long(myIntervalsValues.size()); i++)
        if (myIntervalsHeight.at(i) <= maxHeightInv && (myIntervalsValues.at(i) >= lapseRateT1) && (myIntervalsValues.at(i) > (myIntervalsValues.at(0) + 0.001 * (myIntervalsHeight.at(i) - myIntervalsHeight.at(0)))))
        {
            myProxyOrog->setLapseRateH1(myIntervalsHeight.at(i));
            lapseRateT1 = myIntervalsValues.at(i);
            myProxyOrog->setInversionIsSignificative(true);
        }

    /*! no inversion: try regression with all data */
    if (! myProxyOrog->getInversionIsSignificative())
        return (regressionGeneric(orogProxyPos, false, mySettings));

    /*! create vectors below and above inversion */
    for (i = 0; i < long(interpolationPointList.size()); i++)
        if ((interpolationPointList.at(i)).point->z != NODATA && (interpolationPointList.at(i)).isActive)
        {
            if ((interpolationPointList.at(i)).point->z <= myProxyOrog->getLapseRateH1())
            {
                myData1.push_back((interpolationPointList.at(i)).value);
                myHeights1.push_back(float((interpolationPointList.at(i)).point->z));
            }
            else
            {
                myData2.push_back((interpolationPointList.at(i)).value);
                myHeights2.push_back(float((interpolationPointList.at(i)).point->z));
            }
        }


    /*! create vectors of height intervals below and above inversion */
    for (i = 0; i < long(myIntervalsValues.size()); i++)
        if (myIntervalsHeight.at(i) <= myProxyOrog->getLapseRateH1())
        {
            myIntervalsValues1.push_back(myIntervalsValues.at(i));
            myIntervalsHeight1.push_back(myIntervalsHeight.at(i));
        }
        else
        {
            myIntervalsValues2.push_back(myIntervalsValues.at(i));
            myIntervalsHeight2.push_back(myIntervalsHeight.at(i));
        }


    /*! only positive lapse rate*/
    if (myProxyOrog->getInversionIsSignificative() && myIntervalsValues1.size() == myIntervalsValues.size())
    {
        regressionSimple(orogProxyPos, false, &m, &q, &r2);

        if (r2 >= mySignificativeR2)
        {
            myProxyOrog->setInversionLapseRate(m);
            myProxyOrog->setRegressionR2(r2);
            lapseRateT0 = q;
            lapseRateT1 = q + m * myProxyOrog->getLapseRateH1();
        }
        else
        {
            statistics::linearRegression(myIntervalsHeight1.data(), myIntervalsValues1.data(), (long)myIntervalsHeight1.size(), false, &q, &m, &r2);

            myProxyOrog->setRegressionR2(NODATA);

            if (r2 >= mySignificativeR2)
            {
                lapseRateT0 = q;
                myProxyOrog->setInversionLapseRate(m);
                lapseRateT1 = q + m * myProxyOrog->getLapseRateH1();
            }
            else
            {
                myProxyOrog->setInversionLapseRate(0.);
                lapseRateT0 = myIntervalsValues.at(0);
                lapseRateT1 = lapseRateT0;
            }
        }

        return true;
    }

    /*! check inversion significance */
    statistics::linearRegression(myHeights1.data(), myData1.data(), long(myHeights1.size()), false, &q1, &m1, &r2_values);
    if (myIntervalsValues1.size() > 2)
        statistics::linearRegression(myIntervalsHeight1.data(), myIntervalsValues1.data(), long(myIntervalsHeight1.size()), false, &q, &m, &r2_intervals);
    else
        r2_intervals = 0.;

    /*! inversion is not significant with data neither with intervals */
    if (r2_values < mySignificativeR2Inv && r2_intervals < mySignificativeR2Inv)
    {
        regressionSimple(orogProxyPos, false, &m, &q, &r2);

        /*! case 0: regression with all data much significant */
        if (r2 >= 0.5)
        {
            myProxyOrog->setInversionIsSignificative(false);
            myProxyOrog->setLapseRateH1(NODATA);
            myProxyOrog->setInversionLapseRate(NODATA);
            myProxyOrog->setRegressionSlope(minValue(m, float(0.0)));
            myProxyOrog->setRegressionR2(r2);
            return true;
        }

        /*! case 1: analysis only above inversion, flat lapse rate below */
        myProxyOrog->setInversionLapseRate(0.);
        statistics::linearRegression(myHeights2.data(), myData2.data(), long(myHeights2.size()), false, &q2, &m2, &r2);

        if (myData2.size() >= MIN_REGRESSION_POINTS)
        {
            if (r2 >= mySignificativeR2)
            {
                myProxyOrog->setRegressionSlope(minValue(m2, float(0.0)));
                lapseRateT1 = q2 + myProxyOrog->getLapseRateH1() * myProxyOrog->getRegressionSlope();
                lapseRateT0 = lapseRateT1;
                myProxyOrog->setRegressionR2(r2);
                return true;
            }
            else
            {
                statistics::linearRegression(myIntervalsHeight2.data(), myIntervalsValues2.data(),
                                             int(myIntervalsHeight2.size()), false, &q2, &m2, &r2);
                if (r2 >= mySignificativeR2)
                {
                    myProxyOrog->setRegressionSlope(minValue(m2, float(0.0)));
                    lapseRateT1 = q2 + myProxyOrog->getLapseRateH1() * myProxyOrog->getRegressionSlope();
                    lapseRateT0 = lapseRateT1;
                    return true;
                }
            }
        }

        myProxyOrog->setInversionIsSignificative(false);
        myProxyOrog->setLapseRateH1(NODATA);
        myProxyOrog->setInversionLapseRate(NODATA);
        lapseRateT0 = q;
        lapseRateT1 = NODATA;

        /*! case 2: regression with data */
        regressionSimple(orogProxyPos, false, &m, &q, &r2);

        if (r2 >= mySignificativeR2)
        {
            myProxyOrog->setRegressionSlope(minValue(m, 0));
            lapseRateT0 = q;
            myProxyOrog->setRegressionR2(r2);
            return true;
        }
        else
        {
            lapseRateT0 = myIntervalsValues.at(0);
            if (m > 0.)
                myProxyOrog->setRegressionSlope(0.);
            else
                myProxyOrog->setRegressionSlope(climateLapseRate);

            return true;
        }

    }

    /*! significance analysis */
    statistics::linearRegression(myHeights1.data(), myData1.data(), int(myHeights1.size()), false, &q1, &m1, &r21);
    statistics::linearRegression(myHeights2.data(), myData2.data(), int(myHeights2.size()), false, &q2, &m2, &r22);

    myProxyOrog->setRegressionR2(r22);

    if (r21 >= mySignificativeR2Inv && r22 >= mySignificativeR2)
    {
        if (myHeights2.size() < MIN_REGRESSION_POINTS && m2 > 0.)
        {
            m2 = 0.;
            q2 = lapseRateT1;
        }
        findLinesIntersection(q1, m1, q2, m2, &lapseRateH1_, &lapseRateT1);
        lapseRateT0 = q1;
        myProxyOrog->setInversionLapseRate(m1);
        myProxyOrog->setRegressionSlope(m2);
        myProxyOrog->setLapseRateH1(lapseRateH1_);
        if (myProxyOrog->getLapseRateH1() > maxHeightInv)
        {
            lapseRateT1 = lapseRateT1 - (myProxyOrog->getLapseRateH1() - maxHeightInv) * myProxyOrog->getRegressionSlope();
            myProxyOrog->setLapseRateH1(maxHeightInv);
            myProxyOrog->setInversionLapseRate((lapseRateT1 - lapseRateT0) / (myProxyOrog->getLapseRateH1() - myProxyOrog->getLapseRateH0()));
        }
        return true;
    }
    else if (r21 < mySignificativeR2Inv && r22 >= mySignificativeR2)
    {
        if (myHeights2.size() < MIN_REGRESSION_POINTS && m2 > 0.)
        {
            m2 = 0.;
            q2 = lapseRateT1;
        }

        statistics::linearRegression(myIntervalsHeight1.data(), myIntervalsValues1.data(),
                                     long(myIntervalsHeight1.size()), false, &q, &m, &r2);

        myProxyOrog->setRegressionSlope(m2);
        if (r2 >= mySignificativeR2Inv)
        {
            if (findLinesIntersectionAboveThreshold(q, m, q2, m2, 40, &lapseRateH1_, &lapseRateT1))
            {
                myProxyOrog->setLapseRateH1(lapseRateH1_);
                lapseRateT0 = q;
                myProxyOrog->setInversionLapseRate(m);
                if (myProxyOrog->getLapseRateH1() > maxHeightInv)
                {
                    lapseRateT1 = lapseRateT1 - (myProxyOrog->getLapseRateH1() - maxHeightInv) * myProxyOrog->getRegressionSlope();
                    myProxyOrog->setLapseRateH1(maxHeightInv);
                    myProxyOrog->setInversionLapseRate((lapseRateT1 - lapseRateT0) / (myProxyOrog->getLapseRateH1() - myProxyOrog->getLapseRateH0()));
                }
                return true;
            }
        }
        else
        {
            myProxyOrog->setInversionLapseRate(0.);
            lapseRateT1 = q2 + m2 * myProxyOrog->getLapseRateH1();
            lapseRateT0 = lapseRateT1;
            return true;
        }
    }

    else if (r21 >= mySignificativeR2Inv && r22 < mySignificativeR2)
    {
        lapseRateT0 = q1;
        myProxyOrog->setInversionLapseRate(m1);

        statistics::linearRegression(myIntervalsHeight2.data(), myIntervalsValues2.data(),
                                     long(myIntervalsHeight2.size()), false, &q, &m, &r2);
        if (r2 >= mySignificativeR2)
        {
            myProxyOrog->setRegressionSlope(minValue(m, float(0.)));
            findLinesIntersection(lapseRateT0, myProxyOrog->getInversionLapseRate(), q, myProxyOrog->getRegressionSlope(), &lapseRateH1_, &lapseRateT1);
            myProxyOrog->setLapseRateH1(lapseRateH1_);
        }
        else
        {
            myProxyOrog->setRegressionSlope(climateLapseRate);
            findLinesIntersection(lapseRateT0, myProxyOrog->getInversionLapseRate(), lapseRateT1 - myProxyOrog->getRegressionSlope()* myProxyOrog->getLapseRateH1(), myProxyOrog->getRegressionSlope(), &lapseRateH1_, &lapseRateT1);
            myProxyOrog->setLapseRateH1(lapseRateH1_);
        }
        return true;
    }

    else if (r21 < mySignificativeR2Inv && r22 < mySignificativeR2)
    {
        statistics::linearRegression(myIntervalsHeight1.data(), myIntervalsValues1.data(),
                                     long(myIntervalsHeight1.size()), false, &q, &m, &r2);

        if (r2 >= mySignificativeR2Inv)
        {
            lapseRateT0 = q;
            myProxyOrog->setInversionLapseRate(m);
            lapseRateT1 = q + m * myProxyOrog->getLapseRateH1();
        }
        else
        {
            myProxyOrog->setInversionLapseRate(0.);
            lapseRateT0 = myIntervalsValues.at(0);
            lapseRateT1 = lapseRateT0;
        }

        statistics::linearRegression(myIntervalsHeight2.data(), myIntervalsValues2.data(),
                                     long(myIntervalsHeight2.size()), false, &q, &m, &r2);

        if (r2 >= mySignificativeR2)
        {
            myProxyOrog->setRegressionSlope(minValue(m, 0));
            if (findLinesIntersectionAboveThreshold(lapseRateT0, myProxyOrog->getInversionLapseRate(), q, myProxyOrog->getRegressionSlope(), 40, &lapseRateH1_, &lapseRateT1))
            {
                myProxyOrog->setLapseRateH1(lapseRateH1_);
                return true;
            }
        }
        else
        {
            myProxyOrog->setRegressionSlope(climateLapseRate);
            return true;
        }

    }

    /*! check max lapse rate (20 C / 1000 m) */
    if (myProxyOrog->getRegressionSlope() < -0.02)
        myProxyOrog->setRegressionSlope((float)-0.02);

    myProxyOrog->initializeOrography();
    return (regressionGeneric(orogProxyPos, false, mySettings));

}

float inverseDistanceWeighted(vector <Crit3DInterpolationDataPoint> myPointList)
{
    double sum, sumWeights, weight;

    sum = 0 ;
    sumWeights = 0 ;
    for (int i = 0 ; i < (int)(myPointList.size()); i++)
    {
        Crit3DInterpolationDataPoint* myPoint = &(myPointList.at(i));
        if (myPoint->distance > 0.)
        {
            weight = myPoint->distance / 10000. ;
            weight = fabs(1 / (weight * weight * weight));
            sumWeights += weight;
            sum += myPoint->value * weight;
        }
    }

    if (sumWeights > 0.0)
        return float(sum / sumWeights);
    else
        return NODATA;
}

float gaussWeighted(vector <Crit3DInterpolationDataPoint> myPointList)
{
    double sum, sumWeights, weight;
    double distance, deltaZ;
    double Rd=10;
    double Rz=1;

    sum = 0 ;
    sumWeights = 0 ;
    for (int i = 0 ; i < (int)(myPointList.size()); i++)
    {
        Crit3DInterpolationDataPoint* myPoint = &(myPointList.at(i));
        distance = myPoint->distance / 1000.;
        deltaZ = myPoint->deltaZ / 1000.;
        if (myPoint->distance > 0.)
        {
            weight = 1 - exp(-(distance*distance)/(Rd*Rd)) * exp(-(deltaZ*deltaZ)/(Rz*Rz));
            weight = fabs(1 / (weight * weight * weight));
            sumWeights += weight;
            sum += myPoint->value * weight;
        }
    }

    if (sumWeights > 0.0)
        return float(sum / sumWeights);
    else
        return NODATA;
}

bool checkPrecipitationZero(int* nrPrecNotNull, bool* flatPrecipitation)
{
    *flatPrecipitation = true;
    *nrPrecNotNull = 0;
    float myValue = NODATA;

    for (int i = 0; i < (int)(interpolationPointList.size()); i++)
        if (interpolationPointList.at(i).isActive)
            if (interpolationPointList.at(i).value != NODATA)
                if (interpolationPointList.at(i).value >= PREC_THRESHOLD)
                {
                    if (*nrPrecNotNull > 0 && interpolationPointList.at(i).value != myValue)
                        *flatPrecipitation = false;

                    myValue = interpolationPointList.at(i).value;
                    nrPrecNotNull++;
                }

    return (nrPrecNotNull == 0);
}

void prepareJRC()
{
    vector <Crit3DInterpolationDataPoint> precPoints;

    precBinaryPointList.clear();
    precBinaryPointList.resize(interpolationPointList.size());

    int nrPrec = 0;
    for (int i = 0; i < (int)(interpolationPointList.size()); i++)
        if (interpolationPointList.at(i).isActive)
        {
            precBinaryPointList.at(i) = interpolationPointList.at(i);
            precBinaryPointList.at(i).value = 0;

            if (interpolationPointList.at(i).value >= PREC_THRESHOLD)
            {
                precBinaryPointList.at(i).value = 1;
                nrPrec++;
                precPoints.push_back(interpolationPointList.at(i));
            }
        }

    interpolationPointList.clear();
    interpolationPointList = precPoints;
}

float interpolatePrecStep1()
{
    return inverseDistanceWeighted(precBinaryPointList);
}

float interpolatePrecStep2(Crit3DInterpolationSettings* mySettings)
{
    if (mySettings->getInterpolationMethod() == interpolationMethod::idw)
        return inverseDistanceWeighted(interpolationPointList);
        //return gaussWeighted(interpolationPointList);
    else if (mySettings->getInterpolationMethod() == interpolationMethod::kriging)
        return NODATA;
    else
        return NODATA;
}

float interpolatePrec(Crit3DInterpolationSettings* mySettings)
{
    float myResult;

    if (! mySettings->getUseJRC())
        myResult = interpolatePrecStep2(mySettings);
    else
        if (interpolatePrecStep1() >= PREC_JRC_THRESHOLD)
            myResult = interpolatePrecStep2(mySettings);
        else
            myResult = 0.;

    return ((myResult < 0 && myResult != NODATA) ? 0 : myResult);
}

bool getUseDetrendingVar(meteoVariable myVar)
{
    if (myVar == airTemperature ||
            myVar == airDewTemperature ||
            myVar == dailyAirTemperatureAvg ||
            myVar == dailyAirTemperatureMax ||
            myVar == dailyAirTemperatureMin ||
            myVar == dailyAirDewTemperatureAvg ||
            myVar == dailyAirDewTemperatureMax ||
            myVar == dailyAirDewTemperatureMin)

        return true;
    else
        return false;
}

void detrendPoints(meteoVariable myVar, int pos, Crit3DInterpolationSettings* mySettings)
{
    float detrendValue, proxyValue;
    long myIndex;
    Crit3DInterpolationDataPoint* myPoint;
    Crit3DProxyInterpolation* myProxy;

    if (! getUseDetrendingVar(myVar)) return;

    myProxy = mySettings->getProxy(pos);

    for (myIndex = 0; myIndex < long(interpolationPointList.size()); myIndex++)
    {
        detrendValue = 0;
        myPoint = &(interpolationPointList.at(myIndex));

        proxyValue = myPoint->getProxy(pos);

        if (ProxyVarNames.at(myProxy->getName()) == height)
        {
            if (proxyValue != NODATA)
            {
                float LR_above = myProxy->getRegressionSlope();
                if (myProxy->getInversionIsSignificative())
                {
                    float LR_H1 = myProxy->getLapseRateH1();
                    float LR_H0 = myProxy->getLapseRateH0();
                    float LR_below = myProxy->getInversionLapseRate();

                    if (proxyValue <= LR_H1)
                        detrendValue = maxValue(proxyValue - LR_H0, 0) * LR_below;
                    else
                        detrendValue = ((LR_H1 - LR_H0) * LR_below) + (proxyValue - LR_H1) * LR_above;
                }
                else
                    detrendValue = maxValue(proxyValue, 0) * LR_above;
            }
        }

        else
        {
            if (proxyValue != NODATA)
                if (myProxy->getRegressionR2() >= mySettings->getGenericPearsonThreshold())
                    detrendValue = proxyValue * myProxy->getRegressionSlope();
        }

        myPoint->value -= detrendValue;
    }
}

float retrend(meteoVariable myVar, vector <float> myProxyValues, Crit3DInterpolationSettings* mySettings)
{

    if (! getUseDetrendingVar(myVar)) return 0.;

    float retrendValue = 0.;
    float myProxyValue;
    Crit3DProxyInterpolation* myProxy;

    for (int pos=0; pos<mySettings->getProxyNr(); pos++)
    {
        myProxy = mySettings->getProxy(pos);

        if (myProxy->getIsActive())
        {
            myProxyValue = mySettings->getProxyValue(pos, myProxyValues);

            if (myProxyValue != NODATA)
            {
                float proxySlope = myProxy->getRegressionSlope();

                if (myProxy->getProxyPragaName() == height)
                {
                    if (mySettings->getUseThermalInversion() && myProxy->getInversionIsSignificative())
                    {
                        float LR_H0 = myProxy->getLapseRateH0();
                        float LR_H1 = myProxy->getLapseRateH1();
                        float LR_Below = myProxy->getInversionLapseRate();
                        if (myProxyValue <= LR_H1)
                            retrendValue += (maxValue(myProxyValue - LR_H0, 0) * LR_Below);
                        else
                            retrendValue += ((LR_H1 - LR_H0) * LR_Below) + (myProxyValue - LR_H1) * proxySlope;
                    }
                    else
                        retrendValue += maxValue(myProxyValue, 0) * proxySlope;
                }
                else
                    retrendValue += myProxyValue * proxySlope;
            }
        }
    }

    return retrendValue;
}

bool regressionOrography(meteoVariable myVar, int orogProxyPos, Crit3DInterpolationSettings* mySettings)
{
    if (getUseDetrendingVar(myVar))
    {
        if (mySettings->getUseThermalInversion())
            return regressionOrographyT(myVar, orogProxyPos, true, mySettings);
        else
            return regressionSimpleT(myVar, orogProxyPos, mySettings);
    }
    else
    {
        return regressionGeneric(orogProxyPos, false, mySettings);
    }

}

void detrending(meteoVariable myVar, Crit3DInterpolationSettings* mySettings)
{
    if (! getUseDetrendingVar(myVar)) return;

    int nrProxy = mySettings->getProxyNr();
    Crit3DProxyInterpolation* myProxy;

    for (int pos=0; pos<nrProxy; pos++)
    {
        myProxy = mySettings->getProxy(pos);

        if (myProxy->getProxyPragaName() == height)
        {
            if (regressionOrography(myVar, pos, mySettings))
            {
                myProxy->setIsActive(true);
                detrendPoints(myVar, pos, mySettings);
            }
            else
                myProxy->setIsActive(false);
        }
        else
        {
            if (regressionGeneric(pos, false, mySettings))
            {
                myProxy->setIsActive(true);
                detrendPoints(myVar, pos, mySettings);
            }
            else
                myProxy->setIsActive(false);

        }
    }
}

bool preInterpolation(meteoVariable myVar, Crit3DInterpolationSettings* mySettings)
{
    if (myVar == precipitation || myVar == dailyPrecipitation)
    {
        int nrPrecNotNull;
        bool isFlatPrecipitation;
        if (checkPrecipitationZero(&nrPrecNotNull, &isFlatPrecipitation))
        {
            mySettings->setPrecipitationAllZero(true);
            return true;
        }
        else
            mySettings->setPrecipitationAllZero(false);

        if (mySettings->getUseJRC()) prepareJRC();
    }

    detrending(myVar, mySettings);

    return (true);
}


float interpolateSimple(meteoVariable myVar, std::vector <float> myProxyValues, Crit3DInterpolationSettings* mySettings)
{
    float myResult = NODATA;

    /*! interpolate residuals */
    if (mySettings->getInterpolationMethod() == interpolationMethod::idw)
    {
        myResult = inverseDistanceWeighted(interpolationPointList);
    }
    else if (mySettings->getInterpolationMethod() == interpolationMethod::kriging)
        myResult = NODATA;

    if (myResult != NODATA)
        return (myResult + retrend(myVar, myProxyValues, mySettings));
    else
        return NODATA;
}


float interpolate(meteoVariable myVar, float myX, float myY, float myZ, std::vector <float> myProxyValues, Crit3DInterpolationSettings* mySettings)
{
    if ((myVar == precipitation || myVar == dailyPrecipitation) && mySettings->getPrecipitationAllZero()) return 0.;

    float myResult = NODATA;

    if (mySettings->getInterpolationMethod() == interpolationMethod::idw)
    {
        if ((myVar == precipitation || myVar == dailyPrecipitation) && mySettings->getUseJRC())
            assignDistances(&precBinaryPointList, myX, myY, myZ);

        assignDistances(&interpolationPointList, myX, myY, myZ);
    }

    if (myVar == precipitation || myVar == dailyPrecipitation)
    {
        myResult = interpolatePrec(mySettings);
        if (myResult != NODATA)
            if (!mySettings->getUseJRC() && myResult <= PREC_THRESHOLD) myResult = 0.;
    }
    else
        myResult = interpolateSimple(myVar, myProxyValues, mySettings);

    return myResult;

}

std::vector <float> getProxyValuesXY(gis::Crit3DUtmPoint myPoint, Crit3DInterpolationSettings* mySettings)
{
    std::vector <float> myValues;
    float myValue;
    gis::Crit3DRasterGrid* proxyGrid;

    myValues.resize(mySettings->getProxyNr());

    for (int i=0; i<myValues.size(); i++)
    {
        myValues.at(i) = NODATA;

        proxyGrid = mySettings->getProxy(i)->getGrid();
        if (proxyGrid != NULL && proxyGrid->isLoaded)
        {
            myValue = gis::getValueFromXY(*proxyGrid, myPoint.x, myPoint.y);
            if (myValue != proxyGrid->header->flag)
                myValues.at(i) = myValue;
        }
    }

    return myValues;
}


bool interpolateGridDtm(Crit3DInterpolationSettings* mySettings, gis::Crit3DRasterGrid* myGrid, const gis::Crit3DRasterGrid& myDTM, meteoVariable myVar)
{
    if (! myGrid->initializeGrid(myDTM))
        return (false);

    float myX, myY;

    for (long myRow = 0; myRow < myGrid->header->nrRows ; myRow++)
        for (long myCol = 0; myCol < myGrid->header->nrCols; myCol++)
        {
            gis::getUtmXYFromRowColSinglePrecision(*myGrid, myRow, myCol, &myX, &myY);
            float myZ = myDTM.value[myRow][myCol];
            if (myZ != myGrid->header->flag)
                myGrid->value[myRow][myCol] = interpolate(myVar, myX, myY, myZ, getProxyValuesXY(gis::Crit3DUtmPoint(myX, myY), mySettings), mySettings);
        }

    if (! gis::updateMinMaxRasterGrid(myGrid))
        return (false);

    return (true);
}


// require data loaded in interpolationPointList (use checkAndPassDataToInterpolation function)
bool interpolationRaster(meteoVariable myVar, Crit3DInterpolationSettings *mySettings,
                         const Crit3DTime& myTime, const  gis::Crit3DRasterGrid& myDTM,
                         gis::Crit3DRasterGrid *myRaster, std::string *myError)
{
    // check data presence
    if (interpolationPointList.size() == 0)
    {
        *myError = "No data to interpolate";
        return false;
    }

    // check DTM
    if (! myDTM.isLoaded)
    {
        *myError = "Load DTM before";
        return false;
    }

    // Interpolation settings
    mySettings->setCurrentDate(myTime.date);
    mySettings->setCurrentHour(myTime.getHour());

    // Proxy vars regression and detrend
    if (! preInterpolation(myVar, mySettings))
    {
        *myError = "Interpolation: error in function preInterpolation";
        return false;
    }

    // Interpolate
    if (! interpolateGridDtm(mySettings, myRaster, myDTM, myVar))
    {
        *myError = "Interpolation: error in function interpolateGridDtm";
        return false;
    }

    Crit3DTime t = myTime;
    myRaster->timeString = t.toStdString();
    return true;
}


