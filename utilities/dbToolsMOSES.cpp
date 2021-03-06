#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDate>

#include "commonConstants.h"
#include "dbToolsMOSES.h"
#include "utilities.h"
#include "meteoPoint.h"


/*!
 * \brief readMOSESMeteoData
 * \param query
 * \param myError
 * \return true if data are correctly loaded
 * \note meteoPoint have to be initialized BEFORE function
 */
bool readMOSESDailyData(QSqlQuery *query, Crit3DMeteoPoint *meteoPoint, std::string *myError)
{
    const int MAX_MISSING_DAYS = 3;
    QDate myDate, expectedDate, previousDate;
    Crit3DDate date;

    float tmin = NODATA;
    float tmax = NODATA;
    float tmed = NODATA;
    float prec = NODATA;            // [mm]
    float et0 = NODATA;             // [mm]
    float waterTable = NODATA;      // [m]
    float previousTmin = NODATA;
    float previousTmax = NODATA;
    float previousWaterTable = NODATA;
    int nrMissingData = 0;

    // first date
    query->first();
    myDate = query->value("date").toDate();
    expectedDate = myDate;
    previousDate = myDate.addDays(-1);

    do
    {
        myDate = query->value("date").toDate();

        if (! myDate.isValid())
        {
            *myError = "Wrong date format: " + query->value("date").toString().toStdString();
            return false;
        }

        if (myDate != previousDate)
        {
            if (myDate != expectedDate)
            {
                if (expectedDate.daysTo(myDate) > MAX_MISSING_DAYS)
                {
                    *myError = "Wrong METEO: too many missing data." + expectedDate.toString().toStdString();
                    return false;
                }
                else
                {
                    // fill missing data
                    while (myDate != expectedDate)
                    {
                        tmin = previousTmin;
                        tmax = previousTmax;
                        tmed = (tmin + tmax) * 0.5;
                        prec = 0;
                        et0 = NODATA;
                        waterTable = previousWaterTable;

                        date = getCrit3DDate(expectedDate);
                        meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMin, tmin);
                        meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMax, tmax);
                        meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureAvg, tmed);
                        meteoPoint->setMeteoPointValueD(date, dailyPrecipitation, prec);
                        meteoPoint->setMeteoPointValueD(date, dailyReferenceEvapotranspirationHS, et0);
                        meteoPoint->setMeteoPointValueD(date, dailyWaterTableDepth, waterTable);

                        expectedDate = expectedDate.addDays(1);
                    }
                }
            }

            previousTmax = tmax;
            previousTmin = tmin;
            previousWaterTable = waterTable;

            // mandatory variables
            getValue(query->value("tmin"), &tmin);
            getValue(query->value("tmax"), &tmax);
            getValue(query->value("prec"), &prec);

            // check
            if (prec < 0.f) prec = NODATA;
            if (tmin < -50 || tmin > 40) tmin = NODATA;
            if (tmax < -40 || tmax > 50) tmax = NODATA;

            if ((tmin == NODATA) || (tmax == NODATA) || (prec == NODATA))
            {
                if (nrMissingData < MAX_MISSING_DAYS)
                {
                    if (tmin == NODATA) tmin = previousTmin;
                    if (tmax == NODATA) tmax = previousTmax;
                    if (prec == NODATA) prec = 0;
                    nrMissingData++;
                }
                else
                {
                    *myError = "Wrong METEO: too many missing data " + myDate.toString().toStdString();
                    return false;
                }
            }
            else nrMissingData = 0;

            // NOT mandatory variables

            // TAVG [°C]
            getValue(query->value("tavg"), &tmed);
            if (tmed == NODATA || tmed < -40.f || tmed > 40.f)
                 tmed = (tmin + tmax) * 0.5;

            // ET0 [mm]
            getValue(query->value("etp"), &et0);
            if (et0 < 0.f || et0 > 10.f)
                et0 = NODATA;

            // Watertable depth [m]
            getValue(query->value("watertable"), &waterTable);
            if (waterTable < 0.f) waterTable = NODATA;

            date = getCrit3DDate(myDate);
            if (meteoPoint->obsDataD[0].date.daysTo(date) < meteoPoint->nrObsDataDaysD)
            {
                meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMin, (float)tmin);
                meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMax, (float)tmax);
                meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureAvg, (float)tmed);
                meteoPoint->setMeteoPointValueD(date, dailyPrecipitation, (float)prec);
                meteoPoint->setMeteoPointValueD(date, dailyReferenceEvapotranspirationHS, (float)et0);
                meteoPoint->setMeteoPointValueD(date, dailyWaterTableDepth, waterTable);
            }
            else
            {
                *myError = "Wrong METEO: index out of range.";
                return false;
            }

            previousDate = myDate;
            expectedDate = myDate.addDays(1);
        }

    } while(query->next());

    return true;
}

