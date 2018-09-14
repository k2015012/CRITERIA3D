#ifndef SAVECLIMALAYOUT_H
#define SAVECLIMALAYOUT_H

#include <QString>
#include <QGridLayout>
#include <QtWidgets>



class SaveClimaLayout : public QDialog
{

    Q_OBJECT

    public:
        SaveClimaLayout();

        QString getFirstYear() const;
        void setFirstYear(const QString &value);
        QString getLastYear() const;
        void setLastYear(const QString &value);
        QString getVariable() const;
        void setVariable(const QString &value);
        QString getPeriod() const;
        void setPeriod(const QString &value);
        QString getGenericPeriodStart() const;
        void setGenericPeriodStart(const QString &value);
        QString getGenericPeriodEnd() const;
        void setGenericPeriodEnd(const QString &value);
        QString getGenericNYear() const;
        void setGenericNYear(const QString &value);
        QString getSecondElab() const;
        void setSecondElab(const QString &value);
        QString getElab2Param() const;
        void setElab2Param(const QString &value);
        QString getElab() const;
        void setElab(const QString &value);
        QString getElab1Param() const;
        void setElab1Param(const QString &value);

        void addElab();

    private:

        QVBoxLayout mainLayout;
        QVBoxLayout listLayout;
        QHBoxLayout saveButtonLayout;

        QListView listview;
        QStringListModel model;
        QStringList list;

        QPushButton saveList;
        QPushButton loadList;

        QString firstYear;
        QString lastYear;
        QString variable;
        QString period;

        QString genericPeriodStart;
        QString genericPeriodEnd;
        QString genericNYear;

        QString secondElab;
        QString elab2Param;

        QString elab;
        QString elab1Param;


};


#endif // SAVECLIMALAYOUT_H