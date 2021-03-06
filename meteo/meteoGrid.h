#ifndef METEOGRID_H
#define METEOGRID_H

#include <vector>

    #ifndef METEOPOINT_H
        #include "meteoPoint.h"
    #endif
    #ifndef COMMONCONSTANTS_H
        #include "commonConstants.h"
    #endif

#define GRID_MIN_COVERAGE 0

enum gridAggregationMethod {aggrAvg, aggrMedian, aggrStdDeviation};

const std::map<std::string, gridAggregationMethod> gridAggregationMethodNames = {
  { "average", aggrAvg },
  { "median", aggrMedian },
  { "stddev", aggrStdDeviation }
};

std::string getKeyStringAggregationMethod(gridAggregationMethod value);

    class Crit3DMeteoGridStructure
    {
        public:
            Crit3DMeteoGridStructure();

            std::string name() const;
            void setName(const std::string &name);

            gis::Crit3DGridHeader header() const;
            void setHeader(const gis::Crit3DGridHeader &header);

            int dataType() const;
            void setDataType(int dataType);

            bool isRegular() const;
            void setIsRegular(bool isRegular);

            bool isTIN() const;
            void setIsTIN(bool isTIN);

            bool isUTM() const;
            void setIsUTM(bool isUTM);

            bool isLoaded() const;
            void setIsLoaded(bool isLoaded);

            bool isFixedFields() const;
            void setIsFixedFields(bool isFixedFields);

            bool isHourlyDataAvailable() const;
            void setIsHourlyDataAvailable(bool isHourlyDataAvailable);

            bool isDailyDataAvailable() const;
            void setIsDailyDataAvailable(bool isDailyDataAvailable);

    private:
            std::string _name;
            gis::Crit3DGridHeader _header;

            int _dataType;

            bool _isRegular;
            bool _isTIN;
            bool _isUTM;
            bool _isLoaded;

            bool _isFixedFields;
            bool _isHourlyDataAvailable;
            bool _isDailyDataAvailable;

    };


    class Crit3DMeteoGrid
    {

        public:
            gis::Crit3DRasterGrid dataMeteoGrid;

            Crit3DMeteoGrid();
            ~Crit3DMeteoGrid();

            Crit3DMeteoGridStructure gridStructure() const;
            void setGridStructure(const Crit3DMeteoGridStructure &gridStructure);

            std::vector<std::vector<Crit3DMeteoPoint *> > meteoPoints() const;
            void setMeteoPoints(const std::vector<std::vector<Crit3DMeteoPoint *> > &meteoPoints);

            Crit3DMeteoPoint meteoPoint(int row, int col);
            Crit3DMeteoPoint* meteoPointPointer(int row, int col);

            void setActive(int row, int col, bool active);

            bool isAggregationDefined() const;
            void setIsAggregationDefined(bool isAggregationDefined);

            Crit3DDate firstDate() const;
            void setFirstDate(const Crit3DDate &firstDate);

            Crit3DDate lastDate() const;
            void setLastDate(const Crit3DDate &lastDate);

            bool createRasterGrid();
            void fillMeteoRaster();
            void fillMeteoRasterElabValue();
            void fillMeteoRasterAnomalyValue();

            gis::Crit3DGisSettings getGisSettings() const;
            void setGisSettings(const gis::Crit3DGisSettings &gisSettings);

            void initMeteoPoints(int nRow, int nCol);

            void fillMeteoPoint(int row, int col, std::string code, std::string name, int height, bool active);

            bool fillMeteoPointDailyValue(int row, int col, int numberOfDays, int initialize, Crit3DDate date, meteoVariable variable, float value);

            bool fillMeteoPointCurrentDailyValue(Crit3DDate date, meteoVariable variable);

            bool fillMeteoPointHourlyValue(int row, int col, int numberOfDays, int initialize, Crit3DDate date, int  hour, int minute, meteoVariable variable, float value);

            bool fillMeteoPointCurrentHourlyValue(Crit3DDate date, int hour, int minute, meteoVariable variable);

            bool findMeteoPointFromId(int* row, int* col, std::string code);

            bool getMeteoPointActiveId(int row, int col, std::string *id);

            bool findFirstActiveMeteoPoint(std::string* id, int* row, int* col);

            bool isActiveMeteoPointFromId(std::string id);

            void findGridAggregationPoints(gis::Crit3DRasterGrid* myDTM);

            void assignCellAggregationPoints(int row, int col, gis::Crit3DRasterGrid* myDTM, bool excludeNoData);

            void aggregateMeteoGrid(meteoVariable myVar, frequencyType freq, Crit3DDate date, int  hour, int minute, gis::Crit3DRasterGrid* myDTM, gis::Crit3DRasterGrid dataRaster, gridAggregationMethod elab);

            double aggregateMeteoGridPoint(Crit3DMeteoPoint myPoint, gridAggregationMethod elab);


            bool getIsElabValue() const;
            void setIsElabValue(bool isElabValue);

    private:

            Crit3DMeteoGridStructure _gridStructure;
            std::vector<std::vector<Crit3DMeteoPoint*> > _meteoPoints;
            gis::Crit3DGisSettings _gisSettings;

            bool _isAggregationDefined;
            Crit3DDate _firstDate;
            Crit3DDate _lastDate;
            bool _isElabValue;
    };


#endif // METEOGRID_H
