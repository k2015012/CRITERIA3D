#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QtWidgets>

class QDialogButtonBox;
class QFileInfo;
class QTabWidget;


class GeoTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeoTab();
};

class QualityTab : public QWidget
{
    Q_OBJECT

public:
    explicit QualityTab();
};

class ElaborationTab : public QWidget
{
    Q_OBJECT

public:
    explicit ElaborationTab();

private:
    QLineEdit minimumPercentageEdit;
    QLineEdit rainfallThresholdEdit;
    QLineEdit anomalyPtsMaxDisEdit;
    QLineEdit anomalyPtsMaxDeltaZEdit;
    QLineEdit thomThresholdEdit;
    QCheckBox automaticTmedEdit;
    QCheckBox automaticETPEdit;
    QLineEdit gridMinCoverageEdit;
    QLineEdit transSamaniCoefficientEdit;
    QCheckBox mergeJointStationsEdit;
};

class SettingsDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit SettingsDialog(QSettings *settings);

    private:
        QTabWidget *tabWidget;
        QDialogButtonBox *buttonBox;
        GeoTab* geoTab;
        QualityTab* qualityTab;
        ElaborationTab* elabTab;
};

#endif // SETTINGSDIALOG_H
