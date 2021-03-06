#ifndef METEO_H
#define METEO_H

#include <map>

    #ifndef CRIT3DDATE_H
        #include "crit3dDate.h"
    #endif
    #ifndef CRIT3DCOLOR_H
        #include "color.h"
    #endif

    #define DEF_VALUE_MIN_PERCENTAGE 80
    #define DEF_VALUE_RAINFALL_THRESHOLD 0.2f
    #define DEF_VALUE_THOM_THRESHOLD 24
    #define DEF_VALUE_TRANSMISSIVITY_SAMANI 0.17f

    #define TABLE_METEO_POINTS "point_properties"
    #define FIELD_METEO_POINT "id_point"
    #define FIELD_METEO_DATETIME "date"
    #define FIELD_METEO_VARIABLE "id_variable"
    #define FIELD_METEO_VARIABLE_NAME "variable"

    class Crit3DMeteoSettings
    {
    public:
        Crit3DMeteoSettings();

        float getMinimumPercentage() const;
        void setMinimumPercentage(float value);

        float getRainfallThreshold() const;
        void setRainfallThreshold(float value);

        float getThomThreshold() const;
        void setThomThreshold(float value);

        float getTransSamaniCoefficient() const;
        void setTransSamaniCoefficient(float value);

    private:
        float minimumPercentage;
        float rainfallThreshold;
        float thomThreshold;
        float transSamaniCoefficient;
    };

    enum lapseRateCodeType {primary, secondary, supplemental};

    enum meteoVariable {airTemperature, dailyAirTemperatureMin, dailyAirTemperatureMax, dailyAirTemperatureAvg, dailyAirTemperatureRange,
                    precipitation, dailyPrecipitation,
                    airRelHumidity, dailyAirRelHumidityMin, dailyAirRelHumidityMax, dailyAirRelHumidityAvg,
                    airDewTemperature, dailyAirDewTemperatureMin, dailyAirDewTemperatureMax, dailyAirDewTemperatureAvg,
                    thom, dailyThomMax, dailyThomAvg, dailyThomHoursAbove, dailyThomDaytime, dailyThomNighttime,
                    globalIrradiance, directIrradiance, diffuseIrradiance, reflectedIrradiance, atmTransmissivity,
                    dailyGlobalRadiation, dailyDirectRadiation, dailyDiffuseRadiation, dailyReflectedRadiation,
                    windIntensity, dailyWindIntensityAvg, dailyWindIntensityMax, windDirection, dailyWindDirectionPrevailing,
                    leafWetness, dailyLeafWetness, atmPressure,
                    referenceEvapotranspiration, dailyReferenceEvapotranspirationHS, dailyReferenceEvapotranspirationPM, actualEvaporation,
                    dailyBIC,
                    dailyWaterTableDepth,
                    noMeteoTerrain, noMeteoVar};


    const std::map<std::string, meteoVariable> MapDailyMeteoVar = {
      { "DAILY_TMIN", dailyAirTemperatureMin },
      { "DAILY_TMAX", dailyAirTemperatureMax },
      { "DAILY_TAVG", dailyAirTemperatureAvg },
      { "DAILY_PREC", dailyPrecipitation },
      { "DAILY_RHMIN", dailyAirRelHumidityMin },
      { "DAILY_RHMAX", dailyAirRelHumidityMax },
      { "DAILY_RHAVG", dailyAirRelHumidityAvg },
      { "DAILY_RAD", dailyGlobalRadiation },
      { "DAILY_W_INT_AVG", dailyWindIntensityAvg },
      { "DAILY_W_DIR", dailyWindDirectionPrevailing },
      { "DAILY_W_INT_MAX", dailyWindIntensityMax },
      { "DAILY_ET0_HS", dailyReferenceEvapotranspirationHS },
      { "DAILY_ET0_PM", dailyReferenceEvapotranspirationPM },
      { "DAILY_LEAFW", dailyLeafWetness },
      { "DAILY_TEMPRANGE", dailyAirTemperatureRange },
      { "DAILY_AIRDEW_TMIN", dailyAirDewTemperatureMin },
      { "DAILY_AIRDEW_TMAX", dailyAirDewTemperatureMax },
      { "DAILY_AIRDEW_TAVG", dailyAirDewTemperatureAvg },
      { "DAILY_THOMMAX", dailyThomMax },
      { "DAILY_THOMAVG", dailyThomAvg },
      { "DAILY_THOM_HABOVE", dailyThomHoursAbove },
      { "DAILY_THOM_DAYTIME", dailyThomDaytime },
      { "DAILY_THOM_NIGHTTIME", dailyThomNighttime },
      { "DAILY_DIRECT_RAD", dailyDirectRadiation },
      { "DAILY_DIFFUSE_RAD", dailyDiffuseRadiation },
      { "DAILY_REFLEC_RAD", dailyReflectedRadiation },
      { "DAILY_BIC", dailyBIC },
      { "DAILY_WATER_TABLE_DEPTH", dailyWaterTableDepth }
    };

    const std::map<meteoVariable, std::string> MapDailyMeteoVarToString = {
      { dailyAirTemperatureMin , "Tmin" },
      { dailyAirTemperatureMax, "Tmax"  },
      { dailyAirTemperatureAvg, "Tavg"  },
      { dailyPrecipitation, "Prec" },
      { dailyAirRelHumidityMin, "RHmin" },
      { dailyAirRelHumidityMax, "RHmax" },
      { dailyAirRelHumidityAvg, "RHavg" },
      { dailyGlobalRadiation, "Rad" },
      { dailyWindIntensityAvg, "WIntAvg" },
      { dailyWindDirectionPrevailing, "WDirPrev" },
      { dailyWindIntensityMax, "WIntMax" },
      { dailyReferenceEvapotranspirationHS, "ET0 (HS)" },
      { dailyReferenceEvapotranspirationPM, "ET0 (PM)" },
      { dailyLeafWetness, "LeafWetness" },
      { dailyAirTemperatureRange, "TRange" },
      { dailyAirDewTemperatureMin, "TdMin" },
      { dailyAirDewTemperatureMax, "TdMax" },
      { dailyAirDewTemperatureAvg, "TdAvg" },
      { dailyThomMax, "ThomMax" },
      { dailyThomAvg, "ThomAvg" },
      { dailyThomHoursAbove, "ThomHoursAbove" },
      { dailyThomDaytime, "ThomDaytime" },
      { dailyThomNighttime, "ThomNighttime" },
      { dailyDirectRadiation, "RadDir" },
      { dailyDiffuseRadiation, "RadDiff" },
      { dailyReflectedRadiation, "RadRefl" },
      { dailyBIC, "BIC" },
      { dailyWaterTableDepth, "WaterTableDepth" }
    };

    const std::map<std::string, meteoVariable> MapHourlyMeteoVar = {
      { "TAVG", airTemperature },
      { "PREC", precipitation },
      { "RHAVG", airRelHumidity },
      { "RAD", globalIrradiance },
      { "W_INT_AVG", windIntensity },
      { "W_DIR", windDirection },
      { "ET0", referenceEvapotranspiration },
      { "LEAFW", leafWetness },
      { "AIRDEW_T", airDewTemperature },
      { "THOM", thom },
      { "DIRECT_RAD", directIrradiance },
      { "DIFFUSE_RAD", diffuseIrradiance },
      { "REFLEC_RAD", reflectedIrradiance },
      { "ATM_TRANSMIT", atmTransmissivity },
      { "ATM_PRESSURE", atmPressure },
      { "ACTUAL_EVAPO", actualEvaporation }
    };

    const std::map<int, meteoVariable> MapIdMeteoVar = {
      { 101, airTemperature },
      { 102, precipitation },
      { 103, airRelHumidity },
      { 104, globalIrradiance },
      { 105, windIntensity },
      { 106, windDirection },
      { 151, dailyAirTemperatureMin },
      { 152, dailyAirTemperatureMax },
      { 153, dailyAirTemperatureAvg },
      { 154, dailyPrecipitation },
      { 155, dailyAirRelHumidityMin },
      { 156, dailyAirRelHumidityMax },
      { 157, dailyAirRelHumidityAvg },
      { 158, dailyGlobalRadiation },
      { 159, dailyWindIntensityAvg },
      { 160, dailyWindDirectionPrevailing },
      { 161, dailyWindIntensityMax }
    };



    enum frequencyType {hourly, daily, monthly, noFrequency};

    enum surfaceType   {SurfaceTypeWater, SurfaceTypeSoil, SurfaceTypeCrop};

    class Crit3DClimateParameters
    {
    public:
        Crit3DClimateParameters();
        float tminLapseRate[12];
        float tmaxLapseRate[12];
        float tDewMinLapseRate[12];
        float tDewMaxLapseRate[12];

        float getClimateLapseRate(meteoVariable myVar, Crit3DDate myDate, int myHour);
    };

    float relHumFromTdew(float dewT, float airT);

    float tDewFromRelHum(float rhAir, float airT);

    double ET0_Penman_hourly(double heigth, double normalizedTransmissivity, double globalSWRadiation,
                    double airTemp, double airHum, double windSpeed10);

    double ET0_Penman_daily(int myDOY, float myLatitude, float myPressure, float myTmin, float myTmax,
                            float myTminDayAfter, float myUmed, float myVmed10, float mySWGlobRad);

    double ET0_Hargreaves(double KT, double myLat, int myDoy, double tmax, double tmin);

    float computeThomIndex(float temp, float relHum);

    bool setColorScale(meteoVariable variable, Crit3DColorScale *colorScale);

    frequencyType getFrequency(meteoVariable myVar);

    std::string getVariableString(meteoVariable myVar);

    std::string getKeyStringMeteoMap(std::map<std::string, meteoVariable> map, meteoVariable value);

    meteoVariable getKeyMeteoVarMeteoMap(std::map<meteoVariable,std::string> map, std::string value);

    bool checkLapseRateCode(lapseRateCodeType myType, bool useLapseRateCode, bool useSupplemental);

#endif // METEO_H
