#ifndef COMPUTATIONDIALOG_H
#define COMPUTATIONDIALOG_H

#include <QString>
#include <QSettings>
#include <QGridLayout>
#include <QComboBox>

#include <QtWidgets>
#include "project.h"
#include "mainwindow.h"
#include "anomalyLayout.h"


class ComputationDialog : public QDialog
{

    Q_OBJECT

private:
    bool isAnomaly;
        QString title;
        QSettings* settings;
        QDateEdit currentDay;
        QLabel currentDayLabel;
        QComboBox variableList;
        QLineEdit firstYearEdit;
        QLineEdit lastYearEdit;
        QLabel genericStartLabel;
        QLabel genericEndLabel;
        QLabel nrYearLabel;
        QDateEdit genericPeriodStart;
        QDateEdit genericPeriodEnd;
        QLineEdit nrYear;
        QCheckBox readParam;

        QComboBox periodTypeList;
        QComboBox elaborationList;
        QComboBox secondElabList;
        QLineEdit periodDisplay;

        QLineEdit elab1Parameter;
        QLineEdit elab2Parameter;

        AnomalyLayout* anomaly;

    public:
        ComputationDialog(QSettings *settings, bool isAnomaly);
        void done(bool res);
        void displayPeriod(const QString value);
        void checkYears();
        void changeDate(const QDate newDate);
        void listElaboration(const QString value);
        void listSecondElab(const QString value);
        void activeSecondParameter(const QString value);
        void readParameter(int state);
};


#endif // COMPUTATIONDIALOG_H
