/*-----------------------------------------------------------------------------------
    COPYRIGHT 2016 Fausto Tomei, Gabriele Antolini,
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
-----------------------------------------------------------------------------------*/


#ifndef METEO_H
#define METEO_H

    #ifndef CRIT3DDATE_H
        #include "crit3dDate.h"
    #endif

    #define ALBEDO_WATER 0.05
    #define ALBEDO_SOIL 0.15
    #define ALBEDO_CROP 0.25
    #define ALBEDO_CROP_REFERENCE 0.23

    enum meteoVariable {airTemperature, airTemperatureMin, airTemperatureMax, airTemperatureMean,
                        precipitation, precipitationSum, airHumidity, airHumidityMin, airHumidityMax, airHumidityMean, airDewTemperature,
                        globalIrradiance, directIrradiance, diffuseIrradiance, reflectedIrradiance,
                        globalRadiation, directRadiation, diffuseRadiation, reflectedRadiation,
                        windIntensity, windIntensityMean, windDirection, windDirectionPrevailing,
                        wetnessDuration, atmPressure, atmTransmissivity, potentialEvapotranspiration,
                        realEvaporation, waterTableDepth, noMeteoVar};

    enum surfaceType   {SurfaceTypeWater, SurfaceTypeSoil, SurfaceTypeCrop};

    class Crit3DClimateParameters {
    public:
        Crit3DClimateParameters();
        float tminLapseRate[12];
        float tmaxLapseRate[12];
        float tDewMinLapseRate[12];
        float tDewMaxLapseRate[12];

        float getClimateLapseRate(meteoVariable myVar, Crit3DDate* myDate, int myHour);
    };

    float relHumFromTdew(float dewT, float airT);
    float tDewFromRelHum(float rhAir, float airT);
    double ET0_Penman_hourly(double heigth, double normalizedTransmissivity, double globalSWRadiation,
                    double airTemp, double airHum, double windSpeed10);
    double ET0_Penman_daily(int myDOY, float myLatitude, float myPressure,
                           float myTmin, float myTmax, float myTminDayAfter, float myUmed, float myVmed10, float mySWGlobRad);
    double ET0_Hargreaves(double KT, double myLat, int myDoy, double tmax, double tmin);

#endif // METEO_H