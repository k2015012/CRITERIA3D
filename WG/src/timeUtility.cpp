#include <QDebug>
#include <QFile>
#include <QDateTime>

#include "timeUtility.h"


int numMonthsInPeriod(int myMonth1, int myMonth2)
{

    int numMonthsInPeriod = 0;

    if (myMonth2 >= myMonth1)
        // regular period
        numMonthsInPeriod = (myMonth2 - myMonth1 + 1);
    else
        // irregular period (between years)
        numMonthsInPeriod = (12 - myMonth1 + 1) + myMonth2;

    return numMonthsInPeriod;

}

int getNumDaysInMonth(int myMonth, int myYear)
{
    int days = 0;
    if (myMonth < 1 || myMonth > 12)
    {
        qDebug() << "Error invalid number of month";
        return (-1);
    }

    switch (myMonth)
    {
        case(2):
            if (isLeapYear(myYear))
                days = 29;
            else
                days = 28;
            break;
        case(4):
            days = 30;
            break;
        case(6):
            days = 30;
            break;
        case(9):
            days = 30;
            break;
        case(11):
            days = 30;
            break;
        default:
            days = 31;
    }

    return days;
}

// check if currentMETEO includes the last 9 months before wgDoy1
// if it does returns the number of days equals to 9 months before wgDoy1
int checkLastYearDate(Crit3DDate inputFirstDate, Crit3DDate inputLastDate, int dataLenght, int myPredictionYear, int* wgDoy1)
{

    unsigned int i;
    int totalDay = 0;
    Crit3DDate predictionFirstDate = getDateFromDoy (myPredictionYear, *wgDoy1);
    int predictionMonth = predictionFirstDate.month;
    int monthIndex = 0;

    /*
    qDebug() << "\nObserved lastDate =" << getDateString(inputLastDate);

    qDebug() << "\npredictionFirstDate.year = " << predictionFirstDate.year;
    qDebug() << "predictionFirstDate.month = " << predictionFirstDate.month;
    qDebug() << "predictionFirstDate.day = " << predictionFirstDate.day;
    */

    if ( inputLastDate.addDays(NRDAYSTOLERANCE+1) <  predictionFirstDate )
    {
        qDebug() << "\nObserved days missing are more than NRDAYSTOLERANCE" << NRDAYSTOLERANCE << endl;
        return(-1);
    }

    // shift wgDoy1 if there are missing data
    if (inputLastDate < predictionFirstDate)
        *wgDoy1 = *wgDoy1 - (difference(inputLastDate, predictionFirstDate)) + 1 ;
    else
    {
        if (inputLastDate > predictionFirstDate.addDays(60))
        {
            qDebug() << "Check your XML: you have already observed data" << endl;
            return(-1);
        }
        if (isLeapYear(predictionFirstDate.year))
            *wgDoy1 = (*wgDoy1 + (difference(predictionFirstDate, inputLastDate)) + 1 ) % 366;
        else
            *wgDoy1 = (*wgDoy1 + (difference(predictionFirstDate, inputLastDate)) + 1 ) % 365;
    }

    for (i = 0; i<9; i++)
    {
        monthIndex = (predictionMonth-1)- i;
        if (monthIndex <= 0)
        {
            monthIndex = monthIndex + 12 ;
            myPredictionYear = myPredictionYear - 1;
        }
        totalDay = totalDay + getNumDaysInMonth(monthIndex, myPredictionYear );
    }

    //qDebug() << "totalDay = " << totalDay << endl;
    //qDebug() << "dataLenght = " << dataLenght << endl;

    if ( difference(inputFirstDate, predictionFirstDate) < totalDay || dataLenght < (totalDay-NRDAYSTOLERANCE) )
    {
        // currentMETEO does not include 9 months before wgDoy1 or more than NRDAYSTOLERANCE days missing
        return(-1);
    }

    // data missing, wgDoy1 has been shifted
    if (inputLastDate < predictionFirstDate)
        totalDay = totalDay - (difference(inputLastDate, predictionFirstDate)) + 1 ;
    else
        totalDay = totalDay + (difference(predictionFirstDate, inputLastDate)) + 1 ;

    return totalDay ;
}


bool getDoyFromSeason(QString season, int myPredictionYear, int* wgDoy1, int* wgDoy2)
{
    QString period[12] = {"JFM","FMA","MAM","AMJ","MJJ","JJA","JAS","ASO","SON","OND","NDJ","DJF"};
    unsigned int i = 0;
    int found = 0;
    int myMonth1, myMonth2 = 0;

    for (i = 0; i<12; i++)
    {
        if (season.compare(period[i]) == 0)
        {
            found = 1;
            break;
        }
    }
    if (found == 0)
    {
        qDebug() << "Wrong season" ;
        return false;
    }

    myMonth1 = i + 1;        //first month of my season
    myMonth2 = (i + 3) % 12; //last month of my season

    Crit3DDate predictionFirstDate;
    Crit3DDate predictionLastDate;

    predictionFirstDate.year = myPredictionYear;
    predictionFirstDate.month = myMonth1;
    predictionFirstDate.day = 1;

    *wgDoy1 = getDoyFromDate(predictionFirstDate);
    // if the season is between 2 years
    if (season.compare(period[10]) == 0 || season.compare(period[11]) == 0)
    {
        myPredictionYear = myPredictionYear + 1 ;
    }

    predictionLastDate.year = myPredictionYear;
    predictionLastDate.month = myMonth2;
    predictionLastDate.day = getNumDaysInMonth(myMonth2, myPredictionYear);

    *wgDoy2 = getDoyFromDate(predictionLastDate);

    return true;
}


void fixWgDoy(int wgDoy1, int wgDoy2, int predictionYear, int myYear, int* fixwgDoy1, int* fixwgDoy2)
{
    // check if wgDoy1 and wgDoy2 have been computed starting from a leap year and adjust them for standard years
    if (wgDoy1 < wgDoy2)
    {
        if (isLeapYear(predictionYear) && !isLeapYear(myYear))
        {
            // if wgDoy1 or wgDoy2 > 29th Feb.
            if (wgDoy1 >= 60)
                *fixwgDoy1 = wgDoy1-1;

            if (wgDoy1 >= 60)
                *fixwgDoy2 = wgDoy2-1;
        }
        else if ( !isLeapYear(predictionYear) && isLeapYear(myYear))
        {
            // if wgDoy1 or wgDoy2 > 29th Feb.
            if (wgDoy1 >= 60)
                *fixwgDoy1 = wgDoy1+1;

            if (wgDoy1 >= 60)
                *fixwgDoy2 = wgDoy2+1;
        }
        else
        {
            *fixwgDoy1 = wgDoy1;
            *fixwgDoy2 = wgDoy2;
        }
    }
    else
    {
        if (isLeapYear(predictionYear) && !isLeapYear(myYear))
        {
            // if wgDoy1 > 29th Feb.
            if (wgDoy1 >= 60)
                *fixwgDoy1 = wgDoy1-1;
        }

        else if (isLeapYear(predictionYear+1) && !isLeapYear(myYear))
        {
            // if wgDoy2 > 29th Feb.
            if (wgDoy1 >= 60)
                *fixwgDoy2 = wgDoy2-1;
        }
        else
        {
            *fixwgDoy1 = wgDoy1;
            *fixwgDoy2 = wgDoy2;
        }
    }

}

QString getDateString(Crit3DDate myDate)
{
    QDate date = QDate(myDate.year, myDate.month, myDate.day);
    return date.toString();
}

