#ifndef SOLARRADIATION_H
#define SOLARRADIATION_H

    #ifndef RADIATIONDEFINITIONS_H
        #include "radiationSettings.h"
    #endif
    #ifndef GIS_H
        #include "gis.h"
    #endif

    class Crit3DRadiationMaps
    {
    public:
        gis::Crit3DRasterGrid* latMap;
        gis::Crit3DRasterGrid* lonMap;
        gis::Crit3DRasterGrid* slopeMap;
        gis::Crit3DRasterGrid* aspectMap;
        gis::Crit3DRasterGrid* beamRadiationMap;
        gis::Crit3DRasterGrid* diffuseRadiationMap;
        gis::Crit3DRasterGrid* reflectedRadiationMap;
        gis::Crit3DRasterGrid* globalRadiationMap;
        gis::Crit3DRasterGrid* transmissivityMap;
        gis::Crit3DRasterGrid* clearSkyTransmissivityMap;
        gis::Crit3DRasterGrid* sunRiseMap;
        gis::Crit3DRasterGrid* sunSetMap;
        gis::Crit3DRasterGrid* sunAzimuthMap;
        gis::Crit3DRasterGrid* sunElevationMap;
        gis::Crit3DRasterGrid* sunIncidenceMap;
        gis::Crit3DRasterGrid* sunShadowMap;
        gis::Crit3DRasterGrid* linkeMap;
        gis::Crit3DRasterGrid* albedoMap;

        Crit3DRadiationMaps();
        Crit3DRadiationMaps(const gis::Crit3DRasterGrid& myDtm, const gis::Crit3DGisSettings& myGisSettings);
        ~Crit3DRadiationMaps();

        void clean();

        bool isLoaded;
    };

    namespace radiation
    {
        bool computeSunPosition(float lon, float lat, int myTimezone,
                                int myYear,int myMonth, int myDay,
                                int myHour, int myMinute, int mySecond,
                                float temp, float pressure, float aspect, float slope, TsunPosition *mySunPosition);

        int estimateTransmissivityWindow(Crit3DRadiationSettings* mySettings, const gis::Crit3DRasterGrid& myDtm,
                                         const Crit3DRadiationMaps& myRadiationMaps,
                                         gis::Crit3DPoint* myPoint, Crit3DTime myTime, int timeStepSecond);

        bool computeRadiationGridPresentTime(Crit3DRadiationSettings *mySettings, const gis::Crit3DRasterGrid& myDtm,
                                 Crit3DRadiationMaps* radiationMaps, const Crit3DTime& myCrit3DTime);

        float computePointTransmissivity(Crit3DRadiationSettings *mySettings, const gis::Crit3DPoint& myPoint, Crit3DTime UTCTime, float* measuredRad,
                                         int windowWidth, int timeStepSecond, const gis::Crit3DRasterGrid& myDtm);

        gis::Crit3DRasterGrid* getBeamRadiationMap();
        gis::Crit3DRasterGrid* getDiffuseRadiationMap();
        gis::Crit3DRasterGrid* getReflectedRadiationMap();
        gis::Crit3DRasterGrid* getGlobalRadiationMap();

        bool isGridPointComputable(Crit3DRadiationSettings* mySettings, int row, int col, const gis::Crit3DRasterGrid& myDtm, Crit3DRadiationMaps* radiationMaps);
    }


#endif // SOLARRADIATION_H
