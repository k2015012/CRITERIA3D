#ifndef DBARKIMET_H
#define DBARKIMET_H

#include <QString>
#include <QDate>
#include "variableslist.h"
#include "dbMeteoPoints.h"


#define PREC_ID 250
#define RAD_ID 706

class DbArkimet : public Crit3DMeteoPointsDbHandler
{
    public:
        explicit DbArkimet(QString dbName);
        void dbManager();
        QString queryString;

        QString getVarName(int id);
        QList<int> getDailyVar();
        QList<int> getHourlyVar();
        int getId(QString VarName);
        QList<VariablesList> getVariableProperties(QList<int> id);

        void initStationsDailyTables(QDate startDate, QDate endDate, QStringList stations);
        void initStationsHourlyTables(QDate startDate, QDate endDate, QStringList stations);

        void createTmpTableHourly();
        void deleteTmpTableHourly();
        void createTmpTableDaily();
        void deleteTmpTableDaily();

        //void insertOrUpdate(QString date, QString id_point, int id_variable, QString variable_name, double value, int frequency, QString flag);

        bool saveHourlyData();
        bool saveDailyData(QDate startDate, QDate endDate);

        void appendQueryHourly(QString dateTime, QString idPoint, QString idVariable, QString varName, QString value, QString frequency, bool isFirstData);
        void appendQueryDaily(QString date, QString idPoint, QString idVar, QString value, bool isFirstData);
signals:

    protected slots:

};


#endif // DBARKIMET_H
