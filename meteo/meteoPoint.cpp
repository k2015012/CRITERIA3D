/*!
    \copyright 2016 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by ARPAE Emilia-Romagna

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


#include <malloc.h>
#include <math.h>

#include "commonConstants.h"
#include "meteoPoint.h"


Crit3DMeteoPoint::Crit3DMeteoPoint()
{
    this->dataset = "";
    this->municipality = "";
    this->state = "";
    this->region = "";
    this->province = "";

    // Tpoint
    this->name = "";
    this->id = "";
    this->isUTC = true;
    this->isForecast = false;

    this->aggregationPointsMaxNr = 0;

    this->latitude = NODATA;
    this->longitude = NODATA;
    this->area = NODATA;
    this->latInt = NODATA;
    this->lonInt = NODATA;
    this->isInsideDem = false;

    this->nrObsDataDaysH = 0;
    this->nrObsDataDaysD = 0;
    this->hourlyFraction = 1;

    this->obsDataH = NULL;
    this->obsDataD = NULL;

    this->currentValue = NODATA;
    this->residual = NODATA;

    this->elaboration = NODATA;
    this->anomaly = NODATA;
    this->anomalyPercentage = NODATA;

    this->active = false;
    this->selected = false;

    this->quality = quality::missing_data;

    proxyValues.clear();
    lapseRateCode = primary;
    topographicDistance = NULL;
}


void Crit3DMeteoPoint::initializeObsDataH(int myHourlyFraction, int numberOfDays, const Crit3DDate& firstDate)
{
    this->cleanObsDataH();

    nrObsDataDaysH = numberOfDays;
    hourlyFraction = myHourlyFraction;
    int nrDayValues = hourlyFraction * 24 + 1;
    obsDataH = (TObsDataH *) calloc(numberOfDays, sizeof(TObsDataH));
    quality = quality::missing_data;
    residual = NODATA;

    for (int i = 0; i < numberOfDays; i++)
    {
        obsDataH[i].date = firstDate.addDays(i);
        obsDataH[i].tAir = (float *) calloc(nrDayValues, sizeof(float));
        obsDataH[i].prec = (float *) calloc(nrDayValues, sizeof(float));
        obsDataH[i].rhAir = (float *) calloc(nrDayValues, sizeof(float));
        obsDataH[i].tDew = (float *) calloc(nrDayValues, sizeof(float));
        obsDataH[i].irradiance = (float *) calloc(nrDayValues, sizeof(float));
        obsDataH[i].et0 = (float *) calloc(nrDayValues, sizeof(float));
        obsDataH[i].windInt = (float *) calloc(nrDayValues, sizeof(float));
        obsDataH[i].leafW = (int *) calloc(nrDayValues, sizeof(int));
        obsDataH[i].transmissivity = (float *) calloc(nrDayValues, sizeof(float));
        for (int j = 0; j < nrDayValues; j++)
        {
            obsDataH[i].tAir[j] = NODATA;
            obsDataH[i].prec[j] = NODATA;
            obsDataH[i].rhAir[j] = NODATA;
            obsDataH[i].tDew[j] = NODATA;
            obsDataH[i].irradiance[j] = NODATA;
            obsDataH[i].et0[j] = NODATA;
            obsDataH[i].windInt[j] = NODATA;
            obsDataH[i].leafW[j] = NODATA;
            obsDataH[i].transmissivity[j] = NODATA;
        }
    }

}

void Crit3DMeteoPoint::initializeObsDataD(int numberOfDays, const Crit3DDate& firstDate)
{
    this->cleanObsDataD();

    nrObsDataDaysD = numberOfDays;
    obsDataD = (TObsDataD *) calloc(numberOfDays, sizeof(TObsDataD));

    quality = quality::missing_data;
    residual = NODATA;

    for (int i = 0; i < numberOfDays; i++)
    {
        obsDataD[i].date = firstDate.addDays(i);
        obsDataD[i].tMax = NODATA;
        obsDataD[i].tMin = NODATA;
        obsDataD[i].tAvg = NODATA;
        obsDataD[i].prec = NODATA;
        obsDataD[i].rhMax = NODATA;
        obsDataD[i].rhMin = NODATA;
        obsDataD[i].rhAvg = NODATA;
        obsDataD[i].globRad = NODATA;
        obsDataD[i].et0_hs = NODATA;
        obsDataD[i].et0_pm = NODATA;
        obsDataD[i].windIntAvg = NODATA;
        obsDataD[i].windDirPrev = NODATA;
        obsDataD[i].waterTable = NODATA;
    }
}

void Crit3DMeteoPoint::initializeObsDataM(int numberOfMonths, int month, int year)
{
    this->cleanObsDataM();

    nrObsDataDaysM = numberOfMonths;
    obsDataM = (TObsDataM *) calloc(numberOfMonths, sizeof(TObsDataM));

    quality = quality::missing_data;
    residual = NODATA;
    int addYear = -1;


    for (int i = month; i <= numberOfMonths; i++)
    {
        if (i < 12)
        {
            obsDataM[i]._month = i;
            obsDataM[i]._year = year;
        }
        else if (i%12 == 0)
        {
            addYear = addYear+1;
            obsDataM[i]._month = 12;
            obsDataM[i]._year = year + addYear;
        }
        else
        {
            obsDataM[i]._month = i%12;
            obsDataM[i]._year = year + addYear + 1;
        }

        obsDataM[i].tMax = NODATA;
        obsDataM[i].tMin = NODATA;
        obsDataM[i].tAvg = NODATA;
        obsDataM[i].prec = NODATA;
        obsDataM[i].et0 = NODATA;
        obsDataM[i].globRad = NODATA;

    }
}


void Crit3DMeteoPoint::emptyVarObsDataH(meteoVariable myVar, const Crit3DDate& myDate)
{
    if (! isDateLoadedH(myDate)) return;

    int nrDayValues = hourlyFraction * 24 + 1;
    int i = obsDataH[0].date.daysTo(myDate);
    residual = NODATA;

    if (i>=0 && i<nrObsDataDaysH)
        if (obsDataH[i].date == myDate)
            for (int j = 0; j < nrDayValues; j++)
            {
                if (myVar == airTemperature)
                    obsDataH[i].tAir[j] = NODATA;
                else if (myVar == precipitation)
                    obsDataH[i].prec[j] = NODATA;
                else if (myVar == airRelHumidity)
                    obsDataH[i].rhAir[j] = NODATA;
                else if (myVar == airDewTemperature)
                    obsDataH[i].tDew[j] = NODATA;
                else if (myVar == globalIrradiance)
                    obsDataH[i].irradiance[j] = NODATA;
                else if (myVar == windIntensity)
                    obsDataH[i].windInt[j] = NODATA;
                else if (myVar == leafWetness)
                    obsDataH[i].leafW[j] = NODATA;
                else if (myVar == atmTransmissivity)
                    obsDataH[i].transmissivity[j] = NODATA;
				else if (myVar == referenceEvapotranspiration)
                    obsDataH[i].et0[j] = NODATA;										  
			}
}

void Crit3DMeteoPoint::emptyVarObsDataH(meteoVariable myVar, const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (! isDateIntervalLoadedH(date1, date2)) return;

    int nrDayValues = hourlyFraction * 24 + 1;
    int indexIni = obsDataH[0].date.daysTo(date1);
    int indexFin = obsDataH[0].date.daysTo(date2);
    residual = NODATA;

    for (int i = indexIni; i <= indexFin; i++)
        for (int j = 0; j < nrDayValues; j++)
        {
            if (myVar == airTemperature)
                obsDataH[i].tAir[j] = NODATA;
            else if (myVar == precipitation)
                obsDataH[i].prec[j] = NODATA;
            else if (myVar == airRelHumidity)
                obsDataH[i].rhAir[j] = NODATA;
            else if (myVar == airDewTemperature)
                obsDataH[i].tDew[j] = NODATA;
            else if (myVar == globalIrradiance)
                obsDataH[i].irradiance[j] = NODATA;
            else if (myVar == windIntensity)
                obsDataH[i].windInt[j] = NODATA;
            else if (myVar == leafWetness)
                obsDataH[i].leafW[j] = NODATA;
            else if (myVar == atmTransmissivity)
                obsDataH[i].transmissivity[j] = NODATA;
			else if (myVar == referenceEvapotranspiration)
                obsDataH[i].et0[j] = NODATA;
        }
}

void Crit3DMeteoPoint::emptyVarObsDataD(meteoVariable myVar, const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (! isDateIntervalLoadedH(date1, date2)) return;

    int indexIni = obsDataD[0].date.daysTo(date1);
    int indexFin = obsDataD[0].date.daysTo(date2);
    residual = NODATA;

    for (int i = indexIni; i <= indexFin; i++)
        if (myVar == dailyAirTemperatureMax)
            obsDataD[i].tMax = NODATA;
        else if (myVar == dailyAirTemperatureMin)
            obsDataD[i].tMin = NODATA;
        else if (myVar == dailyAirTemperatureMin)
            obsDataD[i].tAvg = NODATA;
        else if (myVar == dailyPrecipitation)
            obsDataD[i].prec = NODATA;
        else if (myVar == dailyAirRelHumidityMax)
            obsDataD[i].rhMax = NODATA;
        else if (myVar == dailyAirRelHumidityMin)
            obsDataD[i].rhMin = NODATA;
        else if (myVar == dailyAirRelHumidityAvg)
            obsDataD[i].rhAvg = NODATA;
        else if (myVar == dailyGlobalRadiation)
            obsDataD[i].globRad = NODATA;
        else if (myVar == dailyWindIntensityAvg)
            obsDataD[i].windIntAvg = NODATA;
        else if (myVar == dailyReferenceEvapotranspirationHS)
            obsDataD[i].et0_hs = NODATA;
        else if (myVar == dailyReferenceEvapotranspirationPM)
            obsDataD[i].et0_pm = NODATA;
        else if (myVar == dailyLeafWetness)
            obsDataD[i].leafW = NODATA;
}

bool Crit3DMeteoPoint::isDateLoadedH(const Crit3DDate& myDate)
{
    if (nrObsDataDaysH == 0)
        return (false);
    else if (myDate < obsDataH->date || myDate > (obsDataH->date.addDays(nrObsDataDaysH - 1)))
        return (false);
    else
        return (true);
}

bool Crit3DMeteoPoint::isDateIntervalLoadedH(const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (nrObsDataDaysH == 0)
        return false;
    else if (date1 > date2)
        return false;
    else if (date1 < obsDataH->date || date2 > (obsDataH->date.addDays(nrObsDataDaysH - 1)))
        return (false);
    else
        return (true);
}

bool Crit3DMeteoPoint::isDateLoadedD(const Crit3DDate& myDate)
{
    if (nrObsDataDaysD == 0)
        return (false);
    else if (myDate < obsDataD->date || myDate > (obsDataD->date.addDays(nrObsDataDaysD - 1)))
        return (false);
    else
        return (true);
}

bool Crit3DMeteoPoint::isDateIntervalLoadedD(const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (nrObsDataDaysD == 0)
        return false;
    else if (date1 > date2)
        return false;
    else if (date1 < obsDataD->date || date2 > (obsDataD->date.addDays(nrObsDataDaysD - 1)))
        return (false);
    else
        return (true);
}

bool Crit3DMeteoPoint::isDateIntervalLoadedH(const Crit3DTime& timeIni, const Crit3DTime& timeFin)
{
    if (nrObsDataDaysH == 0)
        return false;
    else if (timeIni > timeFin)
        return false;
    else if (obsDataH == NULL)
        return false;
    else if (timeIni.date < obsDataH[0].date || timeFin.date > (obsDataH[0].date.addDays(nrObsDataDaysH - 1)))
        return (false);
    else
        return (true);
}

float Crit3DMeteoPoint::obsDataConsistencyH(meteoVariable myVar, const Crit3DTime& timeIni, const Crit3DTime& timeFin)
{
    if (nrObsDataDaysH == 0)
        return 0.0;
    else if (timeIni > timeFin)
        return 0.0;
    else if (obsDataH == NULL)
        return 0.0;
    else if (timeFin.date < obsDataH[0].date || timeIni.date > (obsDataH[0].date.addDays(nrObsDataDaysH - 1)))
        return 0.0;
    else
    {
        Crit3DTime myTime = timeIni;
        float myValue;
        float myDeltaSeconds = (float)3600.0 / hourlyFraction;
        int counter=0, counterAll=0;
        while (myTime <= timeFin)
        {
            myValue = getMeteoPointValueH(myTime.date, myTime.getHour(), myTime.getMinutes(), myVar);
            if (myValue != NODATA) counter++;
            counterAll++;
            myTime = myTime.addSeconds(myDeltaSeconds);
        }
        return (float(counter)/float(counterAll));
    }

}

void Crit3DMeteoPoint::cleanObsDataH()
{
    quality = quality::missing_data;

    if (nrObsDataDaysH > 0)
    {
        for (int i = 0; i < nrObsDataDaysH; i++)
        {
            free(obsDataH[i].tAir);
            free(obsDataH[i].prec);
            free(obsDataH[i].rhAir);
            free(obsDataH[i].tDew);
            free(obsDataH[i].irradiance);
            free(obsDataH[i].windInt);
            free(obsDataH[i].leafW);
            free(obsDataH[i].transmissivity);
        }
        free (obsDataH);
    }
}


void Crit3DMeteoPoint::cleanObsDataD()
{
    quality = quality::missing_data;

    if (nrObsDataDaysD > 0)
        free (obsDataD);
}

void Crit3DMeteoPoint::cleanObsDataM()
{
    quality = quality::missing_data;

    if (nrObsDataDaysM > 0)
        free (obsDataM);
}

bool Crit3DMeteoPoint::setMeteoPointValueH(const Crit3DDate& myDate, int myHour, int myMinutes, meteoVariable myVar, float myValue)
{
    // check indexes (day, hour)
    int i = obsDataH[0].date.daysTo(myDate);
    if ((i < 0) || (i >= nrObsDataDaysH)) return false;

    int subH = int(ceil(float(myMinutes) / float(60 / hourlyFraction)));
    int h = hourlyFraction * myHour + subH;
    if ((h < 0) || (h >= hourlyFraction * 24)) return false;

    if (myVar == airTemperature)
        obsDataH[i].tAir[h] = myValue;
    else if (myVar == precipitation)
        obsDataH[i].prec[h] = myValue;
    else if (myVar == airRelHumidity)
        obsDataH[i].rhAir[h] = myValue;
    else if (myVar == airDewTemperature)
        obsDataH[i].tDew[h] = myValue;
    else if (myVar == globalIrradiance)
        obsDataH[i].irradiance[h] = myValue;
    else if (myVar == referenceEvapotranspiration)
        obsDataH[i].et0[h] = myValue;
    else if (myVar == windIntensity)
        obsDataH[i].windInt[h] = myValue;
    else if (myVar == leafWetness)
        obsDataH[i].leafW[h] = (int)myValue;
    else if (myVar == atmTransmissivity)
        obsDataH[i].transmissivity[h] = myValue;
    else
        return false;

    // check if is the first day
    if (i != 0)
    {
        if (h == 0)
        {
            // copy 00:00 to 24 day before
            i = i-1;
            h = 24;

            if (myVar == airTemperature)
                obsDataH[i].tAir[h] = myValue;
            else if (myVar == precipitation)
                obsDataH[i].prec[h] = myValue;
            else if (myVar == airRelHumidity)
                obsDataH[i].rhAir[h] = myValue;
            else if (myVar == airDewTemperature)
                obsDataH[i].tDew[h] = myValue;
            else if (myVar == globalIrradiance)
                obsDataH[i].irradiance[h] = myValue;
            else if (myVar == referenceEvapotranspiration)
                obsDataH[i].et0[h] = myValue;
            else if (myVar == windIntensity)
                obsDataH[i].windInt[h] = myValue;
            else if (myVar == leafWetness)
                obsDataH[i].leafW[h] = (int)myValue;
            else if (myVar == atmTransmissivity)
                obsDataH[i].transmissivity[h] = myValue;
            else
                return false;
        }
    }

    return true;
}


bool Crit3DMeteoPoint::setMeteoPointValueD(const Crit3DDate& myDate, meteoVariable myVar, float myValue)
{

    long i = obsDataD[0].date.daysTo(myDate);
    if ((i <0) || (i >= nrObsDataDaysD)) return false;

    if (myVar == dailyAirTemperatureMax)
        obsDataD[i].tMax = myValue;
    else if (myVar == dailyAirTemperatureMin)
        obsDataD[i].tMin = myValue;
    else if (myVar == dailyAirTemperatureAvg)
        obsDataD[i].tAvg = myValue;
    else if (myVar == dailyPrecipitation)
        obsDataD[i].prec = myValue;
    else if (myVar == dailyAirRelHumidityMax)
        obsDataD[i].rhMax = myValue;
    else if (myVar == dailyAirRelHumidityMin)
        obsDataD[i].rhMin = myValue;
    else if (myVar == dailyAirRelHumidityAvg)
        obsDataD[i].rhAvg = myValue;
    else if (myVar == dailyGlobalRadiation)
        obsDataD[i].globRad = myValue;
    else if (myVar == dailyReferenceEvapotranspirationHS)
         obsDataD[i].et0_hs = myValue;
    else if (myVar == dailyReferenceEvapotranspirationPM)
         obsDataD[i].et0_pm = myValue;
    else if (myVar == dailyWindIntensityAvg)
        obsDataD[i].windIntAvg = myValue;
    else if (myVar == dailyWindDirectionPrevailing)
        obsDataD[i].windDirPrev = myValue;
    else if (myVar == dailyWindIntensityMax)
        obsDataD[i].windIntMax = myValue;
    else if (myVar == dailyLeafWetness)
        obsDataD[i].leafW = myValue;					
    else if (myVar == dailyWaterTableDepth)
        obsDataD[i].waterTable = myValue;
    else
        return false;

    return true;
}


float Crit3DMeteoPoint::getMeteoPointValueH(const Crit3DDate& myDate, int myHour, int myMinutes, meteoVariable myVar)
{
    //check
    if (myVar == noMeteoVar)
    {
        return NODATA;
    }
    if (obsDataH == NULL)
    {
        return NODATA;
    }
    if ((myHour < 0) || (myHour > 24))
    {
        return NODATA;
    }

    //indexes
    int subH = int(ceil(float(myMinutes) / float(60 / hourlyFraction)));
    int h = hourlyFraction * myHour + subH;
    if ((h < 0) || (h >= hourlyFraction * 24))
    {
        return NODATA;
    }

    int i = obsDataH[0].date.daysTo(myDate);
    if ((i < 0) || (i >= nrObsDataDaysH))
    {
        return NODATA;
    }

    if (myVar == airTemperature)
        return (obsDataH[i].tAir[h]);
    else if (myVar == precipitation)
        return (obsDataH[i].prec[h]);
    else if (myVar == airRelHumidity)
        return (obsDataH[i].rhAir[h]);
    else if (myVar == airDewTemperature)
    {
        if (obsDataH[i].tDew[h] != NODATA)
            return obsDataH[i].tDew[h];
        else
            return tDewFromRelHum(obsDataH[i].rhAir[h], obsDataH[i].tAir[h]);
    }
    else if (myVar == globalIrradiance)
        return (obsDataH[i].irradiance[h]);
    else if (myVar == referenceEvapotranspiration)
        return (obsDataH[i].et0[h]);
    else if (myVar == windIntensity)
        return (obsDataH[i].windInt[h]);
    else if (myVar == leafWetness)
        return (float)(obsDataH[i].leafW[h]);
    else if (myVar == atmTransmissivity)
        return (obsDataH[i].transmissivity[h]);
    else
    {
        return (NODATA);
    }
}


float Crit3DMeteoPoint::getMeteoPointValueD(const Crit3DDate& myDate, meteoVariable myVar)
{
    //check
    if (myVar == noMeteoVar) return NODATA;
    if (obsDataD == NULL) return NODATA;

    int i = obsDataD[0].date.daysTo(myDate);
    if ((i < 0) || (i >= nrObsDataDaysD)) return NODATA;

    if (myVar == dailyAirTemperatureMax)
        return (obsDataD[i].tMax);
    else if (myVar == dailyAirTemperatureMin)
        return (obsDataD[i].tMin);
    else if (myVar == dailyAirTemperatureAvg)
        return (obsDataD[i].tAvg);
    else if (myVar == dailyPrecipitation)
        return (obsDataD[i].prec);
    else if (myVar == dailyAirRelHumidityMax)
        return (obsDataD[i].rhMax);
    else if (myVar == dailyAirRelHumidityMin)
        return (float)(obsDataD[i].rhMin);
    else if (myVar == dailyAirRelHumidityAvg)
        return (obsDataD[i].rhAvg);
    else if (myVar == dailyGlobalRadiation)
        return (obsDataD[i].globRad);
    else if (myVar == dailyReferenceEvapotranspirationHS)
        return (obsDataD[i].et0_hs);
    else if (myVar == dailyReferenceEvapotranspirationPM)
        return (obsDataD[i].et0_pm);
    else if (myVar == dailyWindIntensityAvg)
        return (obsDataD[i].windIntAvg);
    else if (myVar == dailyWindDirectionPrevailing)
        return (obsDataD[i].windDirPrev);
    else if (myVar == dailyWindIntensityMax)
        return (obsDataD[i].windIntMax);
    else if (myVar == dailyLeafWetness)
        return (obsDataD[i].leafW);
    else if (myVar == dailyWaterTableDepth)
        return (obsDataD[i].waterTable);
    else
        return (NODATA);

}


float Crit3DMeteoPoint::getMeteoPointValue(const Crit3DTime& myTime, meteoVariable myVar)
{
    frequencyType frequency = getFrequency(myVar);
    if (frequency == hourly)
        return getMeteoPointValueH(myTime.date, myTime.getHour(), myTime.getMinutes(), myVar);
    else if (frequency == daily)
        return getMeteoPointValueD(myTime.date, myVar);
    else
        return NODATA;
}

float Crit3DMeteoPoint::getProxyValue(unsigned int pos)
{
    if (pos < proxyValues.size())
        return proxyValues.at(pos);
    else
        return NODATA;
}

std::vector <float> Crit3DMeteoPoint::getProxyValues()
{
    std::vector <float> myValues;
    for (int i=0; i < proxyValues.size(); i++)
        myValues.push_back(getProxyValue(i));

    return myValues;
}
