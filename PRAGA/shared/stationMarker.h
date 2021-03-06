#ifndef STATIONMARKER_H
#define STATIONMARKER_H

#include "MapGraphics_global.h"
#include "CircleObject.h"
#include "MapGraphicsView.h"

    class Crit3DMeteoPoint;
    class Project;

    class StationMarker : public CircleObject
    {
        Q_OBJECT

        public:
            explicit StationMarker(qreal radius, bool sizeIsZoomInvariant, QColor fillColor, MapGraphicsView* view, MapGraphicsObject *parent = 0);
            void setToolTip(Crit3DMeteoPoint* meteoPoint_);

        private:
            MapGraphicsView* _view;

        protected:
            void mousePressEvent(Project *project, QGraphicsSceneMouseEvent *event);
            //void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

    };

#endif // STATIONMARKER_H


