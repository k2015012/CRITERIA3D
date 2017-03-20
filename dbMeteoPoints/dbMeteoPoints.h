#ifndef DBMETEOPOINTS_H
#define DBMETEOPOINTS_H

#include <QString>
#include <QtSql>

#ifndef CRIT3DDATE_H
    #include "crit3dDate.h"
#endif

struct TPointProperties {
    int id;
    QString name;
    QString network;
    double lat;
    double lon;
    int latInt;
    int lonInt;
    double utm_x;
    double utm_y;
    float altitude;
    QString state;
    QString region;
    QString province;
    QString municipality;
};

#define PREC_ID 250
#define RAD_ID 706
#define DAILY_TO_INTEGRAL_RAD 86400

class DbMeteoPoints : public QObject
{
    Q_OBJECT
    public:
        explicit DbMeteoPoints(QString dbName);
        ~DbMeteoPoints();
        void dbManager();
        QString getDatasetURL(QString dataset);
        QStringList getDatasetsActive();
        QString getVarName(int id);
        QList<int> getDailyVar();
        QList<int> getHourlyVar();
        int getId(QString VarName);
        bool fillPointProperties(TPointProperties* pointProp);
        void initStationsTables(Crit3DDate dataStartInput, Crit3DDate dataEndInput, QList<int> stations);
        void insertVarValue(QString station, QString date, int varType, double varValue, QString flag);
        int arkIdmap(int arkId);
    private:
        QSqlDatabase _db;
    signals:

    protected slots:

};


#endif // DBMETEOPOINTS_H
