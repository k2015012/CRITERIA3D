#include <QDebug>
#include <QString>
#include <QPainter>

#include "CircleObject.h"
#include "Position.h"

#include "raster.h"
#include "mainwindow.h"

gis::Crit3DRasterGrid *DTM;
gis::Crit3DGeoMap *geoMap;

using namespace std;


void initializeDTM()
{
    DTM = new gis::Crit3DRasterGrid();
    geoMap = new gis::Crit3DGeoMap();
}


bool loadRaster(QString myFileName, gis::Crit3DRasterGrid *outputRaster)
{
    string* myError = new std::string();
    string fileName = myFileName.left(myFileName.length()-4).toStdString();

    gis::Crit3DRasterGrid utmRaster;
    if (gis::readEsriGrid(fileName, &(utmRaster), myError))
    {
        gis::Crit3DGisSettings mySettings;
        gis::Crit3DGridHeader myLatLonHeader;
        gis::getGeoExtentsFromUTMHeader(mySettings, utmRaster.header, &myLatLonHeader);

        outputRaster->initializeGrid(myLatLonHeader);

        // transform
        double lat, lon, x, y;
        for (long row = 0; row < outputRaster->header->nrRows; row++)
            for (long col = 0; col < outputRaster->header->nrCols; col++)
            {
                gis::getUtmXYFromRowCol(*(outputRaster), row, col, &(lon), &(lat));
                gis::latLonToUtmForceZone(mySettings.utmZone, lat, lon, &x, &y);
                outputRaster->value[row][col] = gis::getValueFromXY(utmRaster, x, y);
            }

        outputRaster->isLoaded = true;
        updateMinMaxRasterGrid(outputRaster);

        // colorscale
        gis::setDefaultDTMScale(outputRaster->colorScale);
        qDebug() << "Raster Ok.";
        return (true);
    }
    else
    {
        qDebug() << "Load raster failed!";
        return (false);
    }
}


bool setMapResolution(MapGraphicsView* view)
{
    if(DTM == NULL) return false;

    QPointF bottomLeft = view->mapToScene(QPoint(0.0,view->height()));
    QPointF topRight = view->mapToScene(QPoint(view->width(),0.0));

    geoMap->bottomLeft.longitude = bottomLeft.x();
    geoMap->bottomLeft.latitude = bottomLeft.y();
    geoMap->topRight.longitude = topRight.x();
    geoMap->topRight.latitude = topRight.y();

    const qreal widthLon = qAbs<qreal>(topRight.x() - bottomLeft.x());
    const qreal heightlat = qAbs<qreal>(topRight.y() - bottomLeft.y());

    qreal dxdegree = widthLon / view->width();
    qreal dydegree = heightlat / view->height();

    geoMap->setResolution(dxdegree, dydegree);

    return true;
}


bool drawRaster(gis::Crit3DRasterGrid* myRaster, gis::Crit3DGeoMap* myMap, QPainter* myPainter)
{
    if ( myRaster == NULL) return false;
    if (! myRaster->isLoaded) return false;

    long row0, row1, col0, col1;
    int x0, y0, x1, y1, lx, ly;
    float myValue;
    gis::Crit3DColor* myColor;
    QColor myQColor;

    gis::getRowColFromXY(*myRaster, myMap->bottomLeft.longitude, myMap->bottomLeft.latitude, &row0, &col0);
    gis::getRowColFromXY(*myRaster, myMap->topRight.longitude, myMap->topRight.latitude, &row1, &col1);

    row1--; col1++;

    row0 = min(myRaster->header->nrRows-1, max(long(0), row0));
    row1 = min(myRaster->header->nrRows-1, max(long(0), row1));
    col0 = min(myRaster->header->nrCols, max(long(0), col0));
    col1 = min(myRaster->header->nrCols, max(long(0), col1));

    gis::updateColorScale(myRaster, row0, row1, col0, col1);

    gis::Crit3DGeoPoint llCorner;
    gis::Crit3DPixel pixelLL;
    llCorner.longitude = myRaster->header->llCorner->x + col0 * myRaster->header->cellSize;
    llCorner.latitude = myRaster->header->llCorner->y + (myRaster->header->nrRows-1 - row0) * myRaster->header->cellSize;
    pixelLL.x = (llCorner.longitude - myMap->referencePoint.longitude) * myMap->degreeToPixelX;
    pixelLL.y = (llCorner.latitude - myMap->referencePoint.latitude) * myMap->degreeToPixelY;

    double dx = myRaster->header->cellSize * myMap->degreeToPixelX;
    double dy = myRaster->header->cellSize * myMap->degreeToPixelY;
    int step = std::max(int(1. / (std::min(dx, dy))), 1);

    y0 = pixelLL.y;
    for (long row = row0; row > row1; row -= step)
    {
        y1 = pixelLL.y + (row0 - row + step) * dy;
        x0 = pixelLL.x;
        for (long col = col0; col < col1; col += step)
        {
            x1 = pixelLL.x + (col-col0 + step) * dx;

            myValue = myRaster->value[row][col];
            if (myValue != myRaster->header->flag)
            {
                myColor = myRaster->colorScale->getColor(myValue);

                myQColor = QColor(myColor->red, myColor->green, myColor->blue);
                myPainter->setPen(myQColor);

                lx = (x1 - x0) +1;
                ly = (y1 - y0) +1;

                // Rectangles
                myPainter->setBrush(myQColor);
                myPainter->fillRect(x0, y0, lx, ly, myPainter->brush());
            }
            x0 = ++x1;
        }
        y0 = ++y1;
    }

    return true;
}



//qreal degreesLatPerMeter(const qreal latitude)
//{
//    const qreal latRad = latitude * (pi / 180.0);
//    qreal meters = 111132.954 - 559.822 * cos(2.0 * latRad) + 1.175 * cos(4.0 * latRad);
//    return 1.0 / meters;
//}

//qreal degreesLonPerMeter(const qreal latitude)
//{
//    const qreal latRad = latitude * (pi / 180.0);
//    qreal meters = (pi * A_EARTH * cos(latRad)) / (180.0 * sqrt(1.0 - NAV_E2 * pow(sin(latRad), 2.0)));
//    return 1.0 / meters;
//}

